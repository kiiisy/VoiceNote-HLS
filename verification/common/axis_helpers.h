#pragma once

#include "../../src/Common/common.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

template <uint16_t MSB, uint16_t LSB>
inline void write_stream(int16_t x, audio::axis_stream_t &s_axis)
{
    ap_uint<audio::AUDIO_SAMPLE_WIDTH> u16 = ap_uint<audio::AUDIO_SAMPLE_WIDTH>((uint16_t)x);

    // 左チャネル
    {
        audio::AxisChannel channel{};
        channel.data                 = 0;
        channel.data.range(MSB, LSB) = u16;
        channel.id                   = audio::AUDIO_LEFT_CHANNEL;
        s_axis.write(channel);
    }
    // 右チャネル
    {
        audio::AxisChannel channel{};
        channel.data                 = 0;
        channel.data.range(MSB, LSB) = u16;
        channel.id                   = audio::AUDIO_RIGHT_CHANNEL;
        s_axis.write(channel);
    }
}

template <uint16_t MSB, uint16_t LSB>
inline int16_t read_stream(audio::axis_stream_t &m_axis)
{
    float out = 0.0f;
    for (uint16_t i = 0; i < audio::AUDIO_CHANNEL; i++) {
        audio::AxisChannel channel = m_axis.read();
        int16_t            s16     = (int16_t)channel.data.range(MSB, LSB).to_uint();
        // 左チャネルのみ抽出する
        if (channel.id == audio::AUDIO_LEFT_CHANNEL) {
            out = s16;
        }
    }

    return out;
}
