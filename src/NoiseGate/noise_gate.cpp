#include "noise_gate.h"

#ifdef __SYNTHESIS__
#include "hls_math.h"
#else
#include <cmath>
#endif

using namespace audio;

static ap_int<AUDIO_SAMPLE_WIDTH> clip(sample_t_ng y)
{
    const sample_t_ng maxv = sample_t_ng(32767);
    const sample_t_ng minv = sample_t_ng(-32768);

    if (y > maxv) {
        y = maxv;
    } else if (y < minv) {
        y = minv;
    }

    sample_t_ng r = (y >= 0) ? (y + sample_t_ng(0.5)) : (y - sample_t_ng(0.5));
    return (ap_int<AUDIO_SAMPLE_WIDTH>)r;
}

static void init_state(NoiseGateState &st)
{
    // ゲイン & ゲートは 0 / false に初期化
    st.gainL      = coef_t_ng(0);
    st.gainR      = coef_t_ng(0);
    st.gate_openL = false;
    st.gate_openR = false;
}

static void core(NoiseGateState &st, const AxisFrame *in_buf, AxisFrame *out_buf, bool ng_pass)
{
    coef_t_ng gainL = st.gainL;
    coef_t_ng gainR = st.gainR;
    bool      gateL = st.gate_openL;
    bool      gateR = st.gate_openR;

    sample_t_ng th_open_amp  = st.th_open_amp;
    sample_t_ng th_close_amp = st.th_close_amp;
    coef_t_ng   a_att        = st.a_attack;
    coef_t_ng   a_rel        = st.a_release;
    coef_t_ng   b_att        = st.b_attack;
    coef_t_ng   b_rel        = st.b_release;

    if (!ng_pass) {
    MAIN_LOOP:
        for (uint16_t i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE II = 5

            AxisFrame sfin = in_buf[i];

            ap_int<AUDIO_SAMPLE_WIDTH> s_in  = sfin.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB);
            sample_t_ng                x     = sample_t_ng(s_in);
            sample_t_ng                level = (x >= 0) ? x : sample_t_ng(-x);

            bool      isL    = (sfin.id == 0);
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

            sample_t_ng y                                        = sample_t_ng(gain * x);
            AxisFrame   sfout                                    = sfin;
            sfout.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB) = clip(y);
            out_buf[i]                                           = sfout;
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

    static NoiseGateState st;
    static bool           inited = false;

    if (!inited) {
        init_state(st);
        inited = true;
    }

    st.th_open_amp  = th_open_amp;
    st.th_close_amp = th_close_amp;
    st.a_attack     = a_attack;
    st.a_release    = a_release;
    st.b_attack     = b_attack;
    st.b_release    = b_release;

    static AxisFrame in_buf[AUDIO_CHANNEL];
    static AxisFrame out_buf[AUDIO_CHANNEL];
//#pragma HLS BIND_STORAGE variable = in_buf type = ram_1p impl = bram
//#pragma HLS BIND_STORAGE variable = out_buf type = ram_1p impl = bram
#pragma HLS BIND_STORAGE variable = in_buf type = fifo impl = srl
#pragma HLS BIND_STORAGE variable = out_buf type = fifo impl = srl

    axis_read_block<AUDIO_CHANNEL>(s_axis, in_buf);
    core(st, in_buf, out_buf, ng_pass);
    axis_write_block<AUDIO_CHANNEL>(out_buf, m_axis);
}
