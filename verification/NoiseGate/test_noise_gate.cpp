#include "../../src/NoiseGate/noise_gate.h"
#include "../common/axis_helpers.h"
#include "../common/test_runner.h"

using namespace testutil;

struct NgFixedParams
{
    audio::sample_t_ng th_open_amp;
    audio::sample_t_ng th_close_amp;
    audio::coef_t_ng   a_attack;
    audio::coef_t_ng   a_release;
    audio::coef_t_ng   b_attack;
    audio::coef_t_ng   b_release;
};

static NgFixedParams make_ng_fixed_params()
{
    const float fs = (float)sample_rate();

    const float open_level   = 0.05f;
    const float close_level  = 0.04f;
    const float attack_time  = 0.005f;
    const float release_time = 0.050f;

    const float th_open     = open_level;
    const float th_close    = close_level;
    const float a_attack_f  = std::exp(-1.0f / (fs * attack_time));
    const float a_release_f = std::exp(-1.0f / (fs * release_time));

    NgFixedParams q{};
    q.th_open_amp  = audio::sample_t_ng(th_open * 32768.0f);
    q.th_close_amp = audio::sample_t_ng(th_close * 32768.0f);

    q.a_attack  = audio::coef_t_ng(a_attack_f);
    q.a_release = audio::coef_t_ng(a_release_f);
    q.b_attack  = audio::coef_t_ng(1.0f - a_attack_f);
    q.b_release = audio::coef_t_ng(1.0f - a_release_f);

    return q;
}

bool run_noise_gate_case(const std::string &case_name, double &rmse_out, int32_t &abs)
{
    return run_case_generic(
        "NoiseGate", case_name,
        [&](audio::axis_stream_t &s, audio::axis_stream_t &m) {
            static NgFixedParams param = make_ng_fixed_params();
            static bool          pass  = false;
            noise_gate(s, m, param.th_open_amp, param.th_close_amp, param.a_attack, param.a_release, param.b_attack,
                       param.b_release, pass);
        },
        rmse_out, abs);
}

#ifdef HLS_TB_STANDALONE  // HLSプロジェクトのCFLAGSで定義

int main()
{
    try {
        struct CaseInfo
        {
            uint16_t    index;  // 使わなくていいかも
            const char *name;
        };

        // GUIで実行する場合はここを編集して
        // 内部の処理の都合、まとめて実行すると不一致が発生する。
        // 見たいテストケースを一つづつコメントアウトで確認する。ダサいが一旦これで。
        // 1と5のテストケースはabsが最大5ぐらいずれる。理由はレベルの変化が大きいから。rmseであっているため一旦無視する。
        const CaseInfo cases[] = {
            {1, "noise_gate_case1"},
            //{2, "noise_gate_case2"},
            //{3, "noise_gate_case3"},
            //{4, "noise_gate_case4"},
            //{5, "noise_gate_case5"},
        };

        bool all_ok = true;
        for (auto &c : cases) {
            double  rmse = 0.0;
            int32_t abs  = 0;
            bool    ok   = run_noise_gate_case(c.name, rmse, abs);
            if (!ok) {
                std::cerr << "[NG] " << c.name << " RMSE=" << rmse << " ABS=" << abs << "\n";
                all_ok = false;
            } else {
                std::cout << "[OK] " << c.name << " RMSE=" << rmse << " ABS=" << abs << "\n";
            }
        }

        return all_ok ? 0 : 1;
    } catch (const std::exception &e) {
        std::cerr << "[EXCEPTION] " << e.what() << "\n";
        return 1;
    }
}

#endif  // HLS_TB_STANDALONE

#ifdef USE_GTEST  // CMakeでunit_testsターゲットで定義

#include <gtest/gtest.h>

static void CheckCase(uint16_t idx)
{
    double      rmse = 0.0;
    std::string name = "noise_gate_case" + std::to_string(idx);

    bool ok = run_noise_gate_case(name, rmse);

    std::cout << "[INFO] " << name << " RMSE=" << rmse << std::endl;

    EXPECT_TRUE(ok) << "RMSE=" << rmse;
}

TEST(NoiseGate, Case1)
{
    CheckCase(1);
}
TEST(NoiseGate, Case2)
{
    CheckCase(2);
}
TEST(NoiseGate, Case3)
{
    CheckCase(3);
}
TEST(NoiseGate, Case4)
{
    CheckCase(4);
}
TEST(NoiseGate, Case5)
{
    CheckCase(5);
}

#endif  // USE_GTEST
