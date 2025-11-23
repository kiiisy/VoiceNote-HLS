#include "dc_cut.h"

using namespace audio;

// 符号付き16bitへの飽和
static ap_int<AUDIO_SAMPLE_WIDTH> clip(sample_t_dc pcm_y)
{
    const sample_t_dc maxv = sample_t_dc(AUDIO_16BIT_MAX);
    const sample_t_dc minv = sample_t_dc(AUDIO_16BIT_MIN);

    // クリップ
    if (pcm_y > maxv) {
        pcm_y = maxv;
    } else if (pcm_y < minv) {
        pcm_y = minv;
    }

    // 丸め込み
    sample_t_dc r = (pcm_y >= 0) ? (pcm_y + sample_t_dc(0.5)) : (pcm_y - sample_t_dc(0.5));

    // 下位整数部を取り出す
    return (ap_int<AUDIO_SAMPLE_WIDTH>)r;
}

static void core(DcBlock &st, const AxisChannel *in_buf, AxisChannel *out_buf, bool dc_pass)
{
    coef_t_ng   a_coeff = st.a_coeff;
    sample_t_dc in_l    = st.in_l;
    sample_t_dc out_l   = st.out_l;
    sample_t_dc in_r    = st.in_r;
    sample_t_dc out_r   = st.out_r;

    if (!dc_pass) {
    MAIN_LOOP:
        for (uint16_t i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE ii = 4

            AxisChannel in_channel = in_buf[i];

            // ストリームデータから16bitのPCMのみ抽出
            ap_int<AUDIO_SAMPLE_WIDTH> pcm_in = in_channel.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB);
            sample_t_dc                pcm_x  = sample_t_dc(pcm_in);
            sample_t_dc                pcm_y;

            if (in_channel.id == AUDIO_LEFT_CHANNEL) {
                // Left チャネル
                pcm_y = pcm_x - in_l + a_coeff * out_l;
                in_l  = pcm_x;
                out_l = pcm_y;
            } else {
                // Right チャネル
                pcm_y = pcm_x - in_r + a_coeff * out_r;
                in_r  = pcm_x;
                out_r = pcm_y;
            }

            AxisChannel out_channel                                    = in_channel;
            out_channel.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB) = clip(pcm_y);
            out_buf[i]                                                 = out_channel;
        }

        // 書き戻す
        st.in_l  = in_l;
        st.out_l = out_l;
        st.in_r  = in_r;
        st.out_r = out_r;
    } else {
    PASS_LOOP:
        for (uint16_t i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE ii = 4
            out_buf[i] = in_buf[i];
        }
    }
}

void dc_cut(axis_stream_t &s_axis, axis_stream_t &m_axis, float a_coef, bool dc_pass)
{
#pragma HLS INTERFACE axis      port = s_axis
#pragma HLS INTERFACE axis      port = m_axis
#pragma HLS INTERFACE s_axilite port = a_coef bundle = dc
#pragma HLS INTERFACE s_axilite port = dc_pass bundle = dc
#pragma HLS INTERFACE s_axilite                port = return bundle = dc
    //#pragma HLS DATAFLOW

    static DcBlock st{coef_t_ng(0), sample_t_dc(0), sample_t_dc(0), sample_t_dc(0), sample_t_dc(0)};

    st.a_coeff = coef_t_ng(a_coef);

    AxisChannel in_buf[AUDIO_CHANNEL];
    AxisChannel out_buf[AUDIO_CHANNEL];
//#pragma HLS BIND_STORAGE variable = in_buf type = ram_s2p impl = bram
//#pragma HLS BIND_STORAGE variable = out_buf type = ram_s2p impl = bram
#pragma HLS BIND_STORAGE variable = in_buf type = fifo impl = lutram
#pragma HLS BIND_STORAGE variable = out_buf type = fifo impl = lutram

    read_stream_buf<AUDIO_CHANNEL>(s_axis, in_buf);
    core(st, in_buf, out_buf, dc_pass);
    write_stream_buf<AUDIO_CHANNEL>(out_buf, m_axis);
}
