#include "Animation.h"

#include <assimp/scene.h>

#include "BoneChannel.h"

namespace animation {
    Animation::Animation(
        const aiAnimation* anim,
        const std::string& uniquePrefix)
        : m_name{ anim->mName.C_Str()},
        m_uniqueName{ uniquePrefix + "_" + m_name },
        m_duration{ static_cast<float>(anim->mDuration) },
        m_ticksPerSecond{ static_cast<float>(anim->mTicksPerSecond) },
        m_index{ -1 }
    {}

    Animation::~Animation() = default;

    animation::BoneChannel& Animation::addChannel(const animation::BoneChannel& src)
    {
        auto& channel = m_channels.emplace_back(src);
        channel.m_index = static_cast<uint16_t>(m_channels.size() - 1);
        return channel;
    }

    void Animation::bindJoint(uint16_t channelIndex, uint16_t jointIndex)
    {
        mjointToChannel.insert({ jointIndex, channelIndex });
        m_channels[channelIndex].m_jointIndex = jointIndex;
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
