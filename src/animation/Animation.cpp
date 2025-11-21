#include "Animation.h"

#include <fmt/format.h>

#include <assimp/scene.h>

#include "RigNodeChannel.h"

namespace animation {
    Animation::Animation(
        const aiAnimation* anim,
        const std::string& uniquePrefix)
        : m_name{ anim->mName.C_Str() },
        m_uniqueName{ fmt::format(
            "{}:{}",
            uniquePrefix.empty() ? "<ANIM_NAME_MISSING>" : uniquePrefix,
            m_name) },
        m_duration{ static_cast<float>(anim->mDuration) },
        m_ticksPerSecond{ static_cast<float>(anim->mTicksPerSecond) },
        m_index{ -1 }
    {}

    Animation::~Animation() = default;

    animation::RigNodeChannel& Animation::addChannel(const animation::RigNodeChannel& src)
    {
        auto& channel = m_channels.emplace_back(src);
        channel.m_index = static_cast<uint16_t>(m_channels.size() - 1);
        return channel;
    }

    void Animation::bindNode(uint16_t channelIndex, uint16_t nodeIndex)
    {
        {
            int oldSize = static_cast<int>(m_nodeToChannel.size());
            int newSize = std::max(static_cast<int>(m_nodeToChannel.size()), nodeIndex + 1);
            if (newSize > oldSize) {
                m_nodeToChannel.resize(newSize);
                for (int i = oldSize; i < newSize; i++) {
                    m_nodeToChannel[i] = -1;
                }
            }
        }

        m_nodeToChannel[nodeIndex] = channelIndex;
        m_channels[channelIndex].m_nodeIndex = nodeIndex;
    }

    uint16_t Animation::getMaxFrame() const
    {
        return m_channels[0].getMaxFrame();
    }

    float Animation::getClipDuration(
        uint16_t firstFrame,
        uint16_t lastFrame) const
    {
        const auto& times = m_channels[0].m_positionKeyTimes;
        return times[lastFrame] - times[firstFrame];
    }
}
