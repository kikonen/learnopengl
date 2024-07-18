#include "AnimationPlay.h"

#include "ki/limits.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"

namespace script
{
    AnimationPlay::AnimationPlay(
        ki::node_id nodeId,
        std::string clip,
        bool repeat) noexcept
        : NodeCommand(nodeId, 0, false),
        m_clip{ clip },
        m_repeat{ repeat }
    {
    }

    void AnimationPlay::execute(
        const UpdateContext& ctx) noexcept
    {
    }
}
