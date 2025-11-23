#include "audio_clean_up.h"

using namespace audio;

inline AxisChannel axis_to_stream(const axis_channel_t &stream)
{
    AxisChannel tmp{};
    tmp.data = stream.data;
    tmp.id   = stream.id;
    return tmp;
}

inline axis_channel_t stream_to_axis(const AxisChannel &tmp)
{
    axis_channel_t stream;
    stream.data = tmp.data;
    stream.id   = tmp.id;
    return stream;
}

void audio_clean_up(axis_port_t &s_axis, axis_port_t &m_axis, float dc_a_coef, bool dc_pass, sample_t_ng th_open_amp,
                    sample_t_ng th_close_amp, coef_t_ng a_attack, coef_t_ng a_release, coef_t_ng b_attack,
                    coef_t_ng b_release, bool ng_pass)
{
#pragma HLS INTERFACE axis port = s_axis
#pragma HLS INTERFACE axis port = m_axis

#pragma HLS INTERFACE s_axilite port = dc_a_coef bundle = acu
#pragma HLS INTERFACE s_axilite port = dc_pass bundle = acu
#pragma HLS INTERFACE s_axilite port = th_open_amp bundle = acu
#pragma HLS INTERFACE s_axilite port = th_close_amp bundle = acu
#pragma HLS INTERFACE s_axilite port = a_attack bundle = acu
#pragma HLS INTERFACE s_axilite port = a_release bundle = acu
#pragma HLS INTERFACE s_axilite port = b_attack bundle = acu
#pragma HLS INTERFACE s_axilite port = b_release bundle = acu
#pragma HLS INTERFACE s_axilite port = ng_pass bundle = acu
#pragma HLS INTERFACE s_axilite                port = return bundle = acu

    axis_stream_t in_stream("in_stream");
    axis_stream_t mid_stream("mid_stream");
    axis_stream_t out_stream("out_stream");
#pragma HLS STREAM variable = in_stream depth = AUDIO_CHANNEL
#pragma HLS STREAM variable = mid_stream depth = AUDIO_CHANNEL
#pragma HLS STREAM variable = out_stream depth = AUDIO_CHANNEL

READ_FROM_PORT:
    for (int i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE II = 1
        axis_channel_t axis   = s_axis.read();
        AxisChannel    stream = axis_to_stream(axis);
        in_stream.write(stream);
    }

    // 1. DCカット
    dc_cut(in_stream, mid_stream, dc_a_coef, dc_pass);

    // 2. ノイズゲート
    noise_gate(mid_stream, out_stream, th_open_amp, th_close_amp, a_attack, a_release, b_attack, b_release, ng_pass);

WRITE_TO_PORT:
    for (int i = 0; i < AUDIO_CHANNEL; ++i) {
#pragma HLS PIPELINE II = 1
        AxisChannel    stream = out_stream.read();
        axis_channel_t axis   = stream_to_axis(stream);
        m_axis.write(axis);
    }
}
