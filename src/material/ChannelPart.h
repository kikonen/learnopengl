#pragma once

#include <string>
#include <array>
#include <type_traits>
#include <stdint.h>

#include "TextureType.h"

struct ChannelPart {
    //
    // How channels of this texture are mapped into result texture channels
    // - max 4 source channels
    // - for each define target channel
    // For example,
    // xxxR = map channel 3 (ALPHA) into target channel 0 (RED)
    enum class Channel : std::underlying_type_t<std::byte> {
        none,
        red,
        green,
        blue,
        alpha
    };

    TextureType m_type;

    std::array<Channel, 4> m_mapping{
        ChannelPart::Channel::none,
        ChannelPart::Channel::none,
        ChannelPart::Channel::none,
        ChannelPart::Channel::none
    };

    static int getChannelIndex(ChannelPart::Channel channel) {
        switch (channel) {
            case ChannelPart::Channel::red:
                return 0;
            case ChannelPart::Channel::green:
                return 1;
            case ChannelPart::Channel::blue:
                return 2;
            case ChannelPart::Channel::alpha:
                return 3;
        }
        return -1;
    }
};
