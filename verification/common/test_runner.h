#pragma once

#include "axis_helpers.h"
#include "test_utils.h"
#include <string>
#include <vector>

template <typename FuncDut>
bool run_case_generic(const std::string &case_name, const std::string &file_name, FuncDut dut, double &rmse_out)
{
    using namespace testutil;

    std::vector<float> mono = load_csv(golden_path(case_name, (file_name + "_input.csv")));
    const size_t       N    = mono.size();
    std::vector<float> out_left(N, 0.0f);

    audio::axis_stream_t s_axis, m_axis;

    for (size_t f = 0; f < N; ++f) {
        // 入力データ作成
        write_stream<audio::AUDIO_SAMPLE_MSB, audio::AUDIO_SAMPLE_LSB>(mono[f], s_axis);

        // メイン処理
        dut(s_axis, m_axis);

        // 出力データ受け取り
        out_left[f] = read_stream<audio::AUDIO_SAMPLE_MSB, audio::AUDIO_SAMPLE_LSB>(m_axis);
    }

    write_csv(output_path(case_name, (file_name + "_hls.csv")), out_left);

    auto golden = load_csv(golden_path(case_name, (file_name + "_cpp.csv")));
    rmse_out    = rmse(out_left, golden);

    return rmse_out < 1e-3;
}
