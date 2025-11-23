#pragma once
#include "../Common/common.h"
#include <cstddef>
#include <cstdint>

// ノイズゲートステート
struct NoiseGateState
{
    // ランタイム状態
    audio::coef_t_ng gainL;
    audio::coef_t_ng gainR;
    bool             gate_openL;
    bool             gate_openR;

    // パラメータ（ホストから渡されるものをそのまま持つ）
    audio::sample_t_ng th_open_amp;   // 開ゲートしきい値（PCMスケール）
    audio::sample_t_ng th_close_amp;  // 閉ゲートしきい値（PCMスケール）
    audio::coef_t_ng   a_attack;
    audio::coef_t_ng   a_release;
    audio::coef_t_ng   b_attack;   // = 1 - a_attack
    audio::coef_t_ng   b_release;  // = 1 - a_release
};

void noise_gate(audio::axis_stream_t &s_axis, audio::axis_stream_t &m_axis, audio::sample_t_ng th_open_amp,
                audio::sample_t_ng th_close_amp, audio::coef_t_ng a_attack, audio::coef_t_ng a_release,
                audio::coef_t_ng b_attack, audio::coef_t_ng b_release, bool ng_pass);
