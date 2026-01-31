#pragma once

#include "../../src/Common/common.h"
#include <algorithm>
#include <cmath>
#include <cstdint>

inline int16_t float_to_s16(float x)
{
    double v = static_cast<double>(x) * 32768.0;
    long   t = std::llround(v);

    if (t > audio::AUDIO_16BIT_MAX) {
        t = audio::AUDIO_16BIT_MAX;
    }
    if (t < audio::AUDIO_16BIT_MIN) {
        t = audio::AUDIO_16BIT_MIN;
    }

    return static_cast<int16_t>(t);
}

inline float s16_to_float(int16_t s)
{
    return static_cast<float>(s) / 32768.0f;
}

template <uint16_t MSB, uint16_t LSB>
inline void write_stream(float x, audio::axis_stream_t &s_axis)
{
    int16_t                            s16 = float_to_s16(x);
    ap_uint<audio::AUDIO_SAMPLE_WIDTH> u16 = ap_uint<audio::AUDIO_SAMPLE_WIDTH>((uint16_t)s16);

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
inline float read_stream(audio::axis_stream_t &m_axis)
{
    float out = 0.0f;
    for (uint16_t i = 0; i < audio::AUDIO_CHANNEL; i++) {
        audio::AxisChannel channel = m_axis.read();
        int16_t            s16     = (int16_t)channel.data.range(MSB, LSB).to_uint();
        // 左チャネルのみ抽出する
        if (channel.id == audio::AUDIO_LEFT_CHANNEL) {
            out = s16_to_float(s16);
        }
    }

    return out;
}
