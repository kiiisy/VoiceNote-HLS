#pragma once
#include "../Common/common.h"
#include <cstddef>
#include <cstdint>

// DCブロック（1次HPF）
struct DcBlock
{
    audio::coef_t_ng   a_coeff;  // フィルタ係数 a
    audio::sample_t_dc in_l;     // 前回入力L
    audio::sample_t_dc out_l;    // 前回出力L
    audio::sample_t_dc in_r;     // 前回入力R
    audio::sample_t_dc out_r;    // 前回出力R
};

void dc_cut(audio::axis_stream_t &s_axis, audio::axis_stream_t &m_axis, float a_coef, bool dc_pass);
