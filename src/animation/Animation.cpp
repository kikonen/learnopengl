#include "Animation.h"

#include <assimp/scene.h>

#include "BoneChannel.h"

namespace animation {
    Animation::Animation(const aiAnimation* anim)
        : m_anim{ anim },
        m_name{ anim->mName.C_Str()  },
        m_duration{ static_cast<float>(anim->mDuration) },
        m_ticksPerSecond{ static_cast<float>(anim->mTicksPerSecond) }
    {}

    Animation::~Animation() = default;

    uint16_t Animation::addChannel(const aiNodeAnim* nodeAnim)
    {
        auto& channel = m_channels.emplace_back(nodeAnim);
        channel.m_id = static_cast<uint16_t>(m_channels.size() - 1);
        return channel.m_id;
    }
}
