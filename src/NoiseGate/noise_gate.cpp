#include "noise_gate.h"

using namespace audio;

namespace {

ap_int<AUDIO_SAMPLE_WIDTH> clip(sample_t_ng pcm_y)
{
    const sample_t_ng maxv = sample_t_ng(AUDIO_16BIT_MAX);
    const sample_t_ng minv = sample_t_ng(AUDIO_16BIT_MIN);

    // クリップ
    if (pcm_y > maxv) {
        pcm_y = maxv;
    } else if (pcm_y < minv) {
        pcm_y = minv;
    }

    // 丸め込み
    sample_t_ng r = (pcm_y >= 0) ? (pcm_y + sample_t_ng(0.5)) : (pcm_y - sample_t_ng(0.5));

    // 下位整数部を取り出す
    return (ap_int<AUDIO_SAMPLE_WIDTH>)r;
}

void core(NoiseGateState &st, NoiseGateParam &param, const AxisChannel *in_buf, AxisChannel *out_buf, bool ng_pass)
{
    coef_t_ng gainL = st.gainL;
    coef_t_ng gainR = st.gainR;
    bool      gateL = st.gate_openL;
    bool      gateR = st.gate_openR;

    sample_t_ng th_open_amp  = param.th_open_amp;
    sample_t_ng th_close_amp = param.th_close_amp;
    coef_t_ng   a_att        = param.a_attack;
    coef_t_ng   a_rel        = param.a_release;
    coef_t_ng   b_att        = param.b_attack;
    coef_t_ng   b_rel        = param.b_release;

    if (!ng_pass) {
    MAIN_LOOP:
        for (uint16_t i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE II = 5

            AxisChannel in_channel = in_buf[i];

            ap_int<AUDIO_SAMPLE_WIDTH> pcm_in = in_channel.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB);
            sample_t_ng                x      = sample_t_ng(pcm_in);
            sample_t_ng                level  = (x >= 0) ? x : sample_t_ng(-x);

            bool      isL    = (in_channel.id == 0);
            coef_t_ng gain   = isL ? gainL : gainR;
            bool      gateon = isL ? gateL : gateR;

            if (gateon) {
                if (level <= th_close_amp) {
                    gateon = false;
                }
            } else {
                if (level >= th_open_amp) {
                    gateon = true;
                }
            }

            coef_t_ng target = gateon ? coef_t_ng(1.0f) : coef_t_ng(0.0f);
            coef_t_ng a      = gateon ? a_att : a_rel;
            coef_t_ng b      = gateon ? b_att : b_rel;

            // g[n] = a*g[n-1] + b*target
            gain = a * gain + b * target;

            if (isL) {
                gainL = gain;
                gateL = gateon;
            } else {
                gainR = gain;
                gateR = gateon;
            }

            sample_t_ng y                                              = sample_t_ng(gain * x);
            AxisChannel out_channel                                    = in_channel;
            out_channel.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB) = clip(y);
            out_buf[i]                                                 = out_channel;
        }

        st.gainL      = gainL;
        st.gate_openL = gateL;
        st.gainR      = gainR;
        st.gate_openR = gateR;
    } else {
    PASS_LOOP:
        for (uint16_t i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE II = 5
            out_buf[i] = in_buf[i];
        }
    }
}
}  // namespace

void noise_gate(axis_stream_t &s_axis, axis_stream_t &m_axis, sample_t_ng th_open_amp, sample_t_ng th_close_amp,
                coef_t_ng a_attack, coef_t_ng a_release, coef_t_ng b_attack, coef_t_ng b_release, bool ng_pass)
{
#pragma HLS INTERFACE axis      port = s_axis
#pragma HLS INTERFACE axis      port = m_axis
#pragma HLS INTERFACE s_axilite port = th_open_amp bundle = ng
#pragma HLS INTERFACE s_axilite port = th_close_amp bundle = ng
#pragma HLS INTERFACE s_axilite port = a_attack bundle = ng
#pragma HLS INTERFACE s_axilite port = a_release bundle = ng
#pragma HLS INTERFACE s_axilite port = b_attack bundle = ng
#pragma HLS INTERFACE s_axilite port = b_release bundle = ng
#pragma HLS INTERFACE s_axilite port = ng_pass bundle = ng
#pragma HLS INTERFACE s_axilite                port = return bundle = ng

    static NoiseGateState st{coef_t_ng(0), coef_t_ng(0), false, false};
    static NoiseGateParam param;

    param.th_open_amp  = th_open_amp;
    param.th_close_amp = th_close_amp;
    param.a_attack     = a_attack;
    param.a_release    = a_release;
    param.b_attack     = b_attack;
    param.b_release    = b_release;

    AxisChannel in_buf[AUDIO_CHANNEL];
    AxisChannel out_buf[AUDIO_CHANNEL];
#pragma HLS BIND_STORAGE variable = in_buf type = fifo impl = lutram
#pragma HLS BIND_STORAGE variable = out_buf type = fifo impl = lutram

    read_stream_buf<AUDIO_CHANNEL>(s_axis, in_buf);
    core(st, param, in_buf, out_buf, ng_pass);
    write_stream_buf<AUDIO_CHANNEL>(out_buf, m_axis);
}
