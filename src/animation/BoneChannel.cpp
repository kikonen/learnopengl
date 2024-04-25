#include "BoneChannel.h"

#include <assimp/scene.h>

#include "util/assimp_util.h"

namespace animation {
    VectorKey::VectorKey(const aiVectorKey& key)
        : m_time{ static_cast<float>(key.mTime) },
        m_value{ assimp_util::toVec3(key.mValue) }
    {}

    QuaternionKey::QuaternionKey(const aiQuatKey& key)
        : m_time{ static_cast<float>(key.mTime) },
        m_value{ assimp_util::toQuat(key.mValue) }
    {}

    BoneChannel::BoneChannel(const aiNodeAnim* channel)
        : m_channel{ channel },
        m_nodeName{ channel->mNodeName.C_Str() }
    {}
}
