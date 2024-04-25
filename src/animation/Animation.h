#pragma once

#include <vector>
#include <string>

struct aiAnimation;

namespace animation {
    struct BoneChannel;

    struct Animation {
        Animation(const aiAnimation* anim);
        ~Animation();

        const std::string m_name;
        const aiAnimation* m_anim;

        float m_duration;
        float m_ticksPerSecond;

        std::vector<animation::BoneChannel> m_channels;
    };
}
