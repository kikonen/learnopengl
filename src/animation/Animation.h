#pragma once

#include <vector>
#include <string>
#include <map>

#include "RigNodeChannel.h"

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
        animation::RigNodeChannel& addChannel(const animation::RigNodeChannel& src);

        void bindNode(uint16_t channelIndex, uint16_t nodeIndex);

        const animation::RigNodeChannel& getChannel(uint16_t index) const noexcept
        {
            return m_channels[index];
        }

        inline const animation::RigNodeChannel* findByNodeIndex(uint16_t nodeIndex) const noexcept
        {
            if (nodeIndex >= m_nodeToChannel.size() - 1) return nullptr;
            const auto index = m_nodeToChannel[nodeIndex];
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

        std::vector<animation::RigNodeChannel> m_channels;

        // map nodeIndex to channel
        // NOTE KI amount of nodes is not ridicilously high thus array should
        // work instead of map (== faster access)
        // -1 == no index
        std::vector<int16_t> m_nodeToChannel;
    };
}
