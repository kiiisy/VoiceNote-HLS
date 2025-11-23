#pragma once

#include <ap_axi_sdata.h>
#include <ap_fixed.h>
#include <ap_int.h>
#include <cstdint>
#include <hls_stream.h>

namespace audio {

static constexpr uint16_t AUDIO_AXIS_DATA_WIDTH = 32;
static constexpr uint16_t AUDIO_AXIS_TID_WIDTH  = 3;
static constexpr uint16_t AUDIO_SAMPLE_MSB      = 27;
static constexpr uint16_t AUDIO_SAMPLE_LSB      = 12;
static constexpr uint16_t AUDIO_SAMPLE_WIDTH    = (AUDIO_SAMPLE_MSB - AUDIO_SAMPLE_LSB) + 1;
static constexpr uint16_t AUDIO_CHANNEL         = 2;
static constexpr uint8_t  AUDIO_LEFT_CHANNEL    = 0;
static constexpr uint8_t  AUDIO_RIGHT_CHANNEL   = 1;
static constexpr uint16_t AUDIO_16BIT_MAX       = 32767;
static constexpr int16_t  AUDIO_16BIT_MIN       = (-32768);

//======================================================================
// ポート用 AXIS 1チャネル型 & ストリーム型
//======================================================================

// AXI4-Stream の「1チャネル」の型
using axis_channel_t = hls::axis<ap_uint<AUDIO_AXIS_DATA_WIDTH>, 0, AUDIO_AXIS_TID_WIDTH, 0>;

// トップポートのストリーム型
using axis_port_t = hls::stream<axis_channel_t>;

//======================================================================
// 内部用フレーム型 & ストリーム
//======================================================================

struct AxisChannel
{
    ap_uint<AUDIO_AXIS_DATA_WIDTH> data;
    ap_uint<AUDIO_AXIS_TID_WIDTH>  id;
};

using axis_stream_t = hls::stream<AxisChannel>;

//======================================================================
// 固定小数点
//======================================================================

// DCカット用
using coef_t_dc   = ap_fixed<16, 1>;
using sample_t_dc = ap_fixed<34, 19>;

// ノイズゲート用
using coef_t_ng   = ap_fixed<24, 2>;
using sample_t_ng = ap_fixed<34, 19>;

//======================================================================
// サンプルヘルパ (内部 AxisChannel 用)
//======================================================================

inline ap_int<AUDIO_SAMPLE_WIDTH> axis_get_sample(const AxisChannel &stream)
{
    return (ap_int<AUDIO_SAMPLE_WIDTH>)stream.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB);
}

inline void axis_set_sample(AxisChannel &stream, ap_int<AUDIO_SAMPLE_WIDTH> data)
{
    stream.data.range(AUDIO_SAMPLE_MSB, AUDIO_SAMPLE_LSB) = (ap_uint<AUDIO_SAMPLE_WIDTH>)static_cast<uint16_t>(data);
}

inline void frame_set_sample(AxisChannel &f, ap_int<AUDIO_SAMPLE_WIDTH> s)
{
    axis_set_sample(f, s);
}

//======================================================================
// ブロック読み書きユーティリティ（内部 AxisChannel ストリーム用）
//======================================================================

template <uint16_t N>
void read_stream_buf(axis_stream_t &s_axis, AxisChannel *buf)
{
READ_LOOP:
    for (uint16_t i = 0; i < N; ++i) {
#pragma HLS PIPELINE ii = 1
        if (s_axis.empty()) {
            break;
        }
        buf[i] = s_axis.read();
    }
}

template <uint16_t N>
void write_stream_buf(const AxisChannel *buf, axis_stream_t &m_axis)
{
WRITE_LOOP:
    for (uint16_t i = 0; i < N; ++i) {
#pragma HLS PIPELINE ii = 1
        m_axis.write(buf[i]);
    }
}

}  // namespace audio
