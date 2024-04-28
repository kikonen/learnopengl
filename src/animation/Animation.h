#pragma once

#include <vector>
#include <string>

#include "BoneChannel.h"

struct aiAnimation;
struct aiNodeAnim;

namespace animation {
    struct Animation {
        Animation(const aiAnimation* anim);
        ~Animation();

        uint16_t addChannel(const aiNodeAnim* nodeAnim);

        animation::BoneChannel& getChannel(uint16_t index) noexcept {
            return m_channels[index];
        }

        const std::string m_name;
        const aiAnimation* m_anim;

        const float m_duration;
        const float m_ticksPerSecond;

        std::vector<animation::BoneChannel> m_channels;
    };
}
