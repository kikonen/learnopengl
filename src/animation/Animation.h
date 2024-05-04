#pragma once

#include <vector>
#include <string>
#include <map>

#include "BoneChannel.h"

struct aiAnimation;

namespace animation {
    struct Animation {
        Animation(
            const aiAnimation* anim,
            const std::string& namePrefix);

        ~Animation();

        // @return channel index
        animation::BoneChannel& addChannel(const animation::BoneChannel& src);

        void bindNode(uint16_t channelIndex, uint16_t nodeIndex);

        const animation::BoneChannel& getChannel(uint16_t index) const noexcept
        {
            return m_channels[index];
        }

        inline const animation::BoneChannel* findByNodeIndex(uint16_t nodeIndex) const noexcept
        {
            const auto& it = mNodeToChannel.find(nodeIndex);
            return it != mNodeToChannel.end() ? &m_channels[it->second] : nullptr;
        }

        const std::string m_name;
        const aiAnimation* m_anim;

        const float m_duration;
        const float m_ticksPerSecond;

        std::vector<animation::BoneChannel> m_channels;
        std::map<uint16_t, uint16_t> mNodeToChannel;
    };
}
