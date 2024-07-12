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

        void bindJoint(uint16_t channelIndex, uint16_t jointIndex);

        const animation::BoneChannel& getChannel(uint16_t index) const noexcept
        {
            return m_channels[index];
        }

        inline const animation::BoneChannel* findByJointIndex(uint16_t jointIndex) const noexcept
        {
            const auto& it = mjointToChannel.find(jointIndex);
            return it != mjointToChannel.end() ? &m_channels[it->second] : nullptr;
        }

        uint16_t getMaxFrame() const;

        uint16_t getClipDuration(
            uint16_t firstFrame,
            uint16_t lastFrame) const;

        const std::string m_name;
        const std::string m_uniqueName;

        const float m_duration;
        const float m_ticksPerSecond;

        int16_t m_index;

        std::vector<animation::BoneChannel> m_channels;
        std::map<uint16_t, uint16_t> mjointToChannel;
    };
}
