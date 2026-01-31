#include "../../src/AudioCleanUp/audio_clean_up.h"
#include "../../src/Common/common.h"
#include "../common/test_utils.h"

#include <cmath>
#include <stdexcept>
#include <string>
#include <vector>

using namespace audio;
using namespace testutil;

// ===== NoiseGate パラメータ生成（HLS noise_gate TB と同じロジック） =====
static void make_ng_params_float(float &th_open, float &th_close, float &a_attack, float &a_release)
{
    const float fs = static_cast<float>(sample_rate());

    const float open_level = 0.05f;
    const float close_level = 0.04f;
    const float attack_time = 0.005f;
    const float release_time = 0.050f;

    th_open = open_level;
    th_close = close_level;

    a_attack = std::exp(-1.0f / (fs * attack_time));
    a_release = std::exp(-1.0f / (fs * release_time));
}

// ===== mono → ステレオ AXI-Stream(L/R同一) 1フレーム変換 =====
// frame_idx の 1 サンプルを L( id=0 ), R( id=1 ) の 2 ワードとして積む
static void mono_frame_to_axis_stereo_port(const std::vector<int16_t> &mono, std::size_t frame_idx, axis_port_t &s_axis)
{

    int16_t sL16 = mono[frame_idx];
    int16_t sR16 = mono[frame_idx]; // L=R

    ap_uint<16> uL16 = (ap_uint<16>)(uint16_t)sL16;
    ap_uint<16> uR16 = (ap_uint<16>)(uint16_t)sR16;

    // Left (id=0)
    {
        axis_channel_t word{};
        word.data = 0;
        word.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB) = uL16;
        word.id = 0;
        s_axis.write(word);
    }

    // Right (id=1)
    {
        axis_channel_t word{};
        word.data = 0;
        word.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB) = uR16;
        word.id = 1;
        s_axis.write(word);
    }
}

// ===== AXI-Stream → Left チャンネル float（1フレーム分） =====
// audio_clean_up 呼び出し 1 回で L/R 2 ワード出てくる前提。
// そのうち id==0 (Left) だけ out_left[frame_idx] に保存。
static void axis_frame_to_left_port(axis_port_t &m_axis, std::size_t frame_idx, std::vector<int16_t> &out_left)
{
    // 2 ワード読む（L + R）
    for (int i = 0; i < 2; ++i)
    {
        if (m_axis.empty())
        {
            throw std::runtime_error("axis_frame_to_left_port: unexpected empty stream");
        }

        axis_channel_t v = m_axis.read();
        ap_uint<32> w = v.data;
        int16_t s16 = (int16_t)w.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB).to_uint();

        if (v.id == 0)
        { // Left のみ保存
            if (frame_idx >= out_left.size())
            {
                throw std::runtime_error("axis_frame_to_left_port: out_left index overflow");
            }
            out_left[frame_idx] = s16;
        }
    }
}

// ===== 1ケース分の実行 =====
static bool run_case_audio_clean_up(int case_index, const std::string &case_name, double &rmse_out)
{
    // 1. 入力読み込み
    std::vector<int16_t> x_mono;
    {
        std::string file;
        switch (case_index)
        {
        case 1:
            file = "audio_clean_up_case1_input.csv";
            break;
        case 2:
            file = "audio_clean_up_case2_input.csv";
            break;
        case 3:
            file = "audio_clean_up_case3_input.csv";
            break;
        default:
            throw std::runtime_error("Invalid case index");
        }
        auto path = golden_path("AudioCleanUp", file);
        x_mono = load_csv(path);
    }

    const std::size_t total_frames = x_mono.size(); // 48000 想定

    // 2. 出力バッファ（Left チャンネル最終波形）
    std::vector<int16_t> out_left(total_frames, 0);

    // 3. パラメータ計算
    float th_open_f, th_close_f, a_attack_f, a_release_f;
    make_ng_params_float(th_open_f, th_close_f, a_attack_f, a_release_f);

    // dc_cut の係数（HPFの a）
    const float dc_a_coef = std::exp(-2.0f * static_cast<float>(M_PI) * 20.0f / static_cast<float>(sample_rate()));

    // HLS IP に渡す固定小数点パラメータ
    sample_t_ng th_open_amp = sample_t_ng(th_open_f * (1 << 15));
    sample_t_ng th_close_amp = sample_t_ng(th_close_f * (1 << 15));

    coef_t_ng a_attack = coef_t_ng(a_attack_f);
    coef_t_ng a_release = coef_t_ng(a_release_f);
    coef_t_ng b_attack = coef_t_ng(1.0f - a_attack_f);
    coef_t_ng b_release = coef_t_ng(1.0f - a_release_f);
    bool dc_pass = false;
    bool ng_pass = false;

    // 4. フレームごとに audio_clean_up を呼び出し（1フレーム=2ワード）
    for (std::size_t f = 0; f < total_frames; ++f)
    {
        axis_port_t s_axis;
        axis_port_t m_axis;

        // このフレームの L/R を積む
        mono_frame_to_axis_stereo_port(x_mono, f, s_axis);

        // トップ関数呼び出し
        audio_clean_up(s_axis, m_axis, dc_a_coef, dc_pass, th_open_amp, th_close_amp, a_attack, a_release, b_attack,
                       b_release, ng_pass);

        // 出力を回収（Left のみ）
        axis_frame_to_left_port(m_axis, f, out_left);
    }

    // 5. 出力を CSV に保存（デバッグ用）
    auto out_path = output_path("AudioCleanUp", case_name + "_hls.csv");
    write_csv(out_path, out_left);

    // 6. ゴールデン読み込み
    auto golden_file = golden_path("AudioCleanUp", case_name + "_cpp.csv");
    auto golden = load_csv(golden_file);

    if (golden.size() != out_left.size())
    {
        std::cerr << "[ERROR] " << case_name << ": size mismatch golden=" << golden.size() << " out=" << out_left.size()
                  << std::endl;
        return false;
    }

    // 7. RMSE 比較
    rmse_out = rmse(out_left, golden);
    const double thresh = 1e-3; // Q15 + dc_cut なので少し余裕を見てもOK

    if (rmse_out >= thresh)
    {
        std::cerr << "[NG] " << case_name << ": RMSE=" << rmse_out << " (thresh=" << thresh << ")" << std::endl;
        return false;
    }

    std::cout << "[OK] " << case_name << ": RMSE=" << rmse_out << std::endl;
    return true;
}

// ===== メイン =====
int main()
{
    bool all_ok = true;
    double rmse = 0.0;

    all_ok &= run_case_audio_clean_up(1, "audio_clean_up_case1", rmse);
    // all_ok &= run_case_audio_clean_up(2, "audio_clean_up_case2", rmse);
    // all_ok &= run_case_audio_clean_up(3, "audio_clean_up_case3", rmse);

    if (!all_ok)
    {
        std::cerr << "[SOME NG] AudioCleanUp HLS vs Ref" << std::endl;
        return 1;
    }

    std::cout << "[ALL OK] AudioCleanUp HLS vs Ref" << std::endl;
    return 0;
}
