#pragma once

#include "../Common/common.h"

#include "../DcCut/dc_cut.h"
#include "../NoiseGate/noise_gate.h"

// トップ関数
void audio_clean_up(audio::axis_port_t &s_axis, audio::axis_port_t &m_axis,
                    //  DCカット用
                    float dc_a_coef, bool dc_pass,
                    //  NoiseGate用（fixed パラメータをそのまま渡す）
                    audio::sample_t_ng th_open_amp, audio::sample_t_ng th_close_amp, audio::coef_t_ng a_attack,
                    audio::coef_t_ng a_release, audio::coef_t_ng b_attack, audio::coef_t_ng b_release, bool ng_pass);
