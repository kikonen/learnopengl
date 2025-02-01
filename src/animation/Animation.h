#pragma once

#include <vector>
#include <string>
#include <map>

#include "BoneChannel.h"

struct aiAnimation;

namespace animation {
    struct Animation {
        friend class AnimationLoader;
        friend struct RigContainer;
        friend struct ClipContainer;

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
            if (jointIndex >= m_jointToChannel.size() - 1) return nullptr;
            const auto index = m_jointToChannel[jointIndex];
            if (index < 0) return nullptr;
            return &m_channels[index];
        }

        uint16_t getMaxFrame() const;

        int16_t getIndex() const noexcept
        {
            return m_index;
        }

        uint16_t getClipCount() const noexcept
        {
            return m_clipCount;
        }

        float getClipDuration(
            uint16_t firstFrame,
            uint16_t lastFrame) const;

        const std::string m_name;
        const std::string m_uniqueName;

        const float m_duration;
        const float m_ticksPerSecond;

    private:
        int16_t m_index;
        int16_t m_clipCount{ 0 };

        std::vector<animation::BoneChannel> m_channels;

        // map jointIndex to channel
        // NOTE KI amount of joints is not ridicilously high thus array should
        // work instead of map (== faster access)
        // -1 == no index
        std::vector<int16_t> m_jointToChannel;
    };
}
