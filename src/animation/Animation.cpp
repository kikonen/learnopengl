#include "Animation.h"

#include <assimp/scene.h>

#include "BoneChannel.h"

namespace animation {
    Animation::Animation(
        const aiAnimation* anim,
        const std::string& namePrefix)
        : m_name{ namePrefix + "_" + anim->mName.C_Str()},
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

    void Animation::bindNode(uint16_t channelIndex, uint16_t nodeIndex)
    {
        mNodeToChannel.insert({ nodeIndex, channelIndex });
        m_channels[channelIndex].m_nodeIndex = nodeIndex;
    }
}
