#include "AnimationPlay.h"

#include "ki/limits.h"

#include "util/debug.h"

#include "model/Node.h"

#include "animation/RigContainer.h"
#include "animation/AnimationSystem.h"

#include "mesh/MeshType.h"
#include "mesh/Mesh.h"

#include "engine/UpdateContext.h"

#include "audio/AudioEngine.h"

#include "registry/Registry.h"

namespace script
{
    AnimationPlay::AnimationPlay(
        pool::NodeHandle handle,
        std::string clipName,
        float speed,
        bool repeat) noexcept
        : NodeCommand(handle, 0, false),
        m_clipName{ clipName },
        m_speed{ speed },
        m_repeat{ repeat }
    {
    }

    void AnimationPlay::bind(const UpdateContext& ctx) noexcept
    {
        NodeCommand::bind(ctx);

        auto* type = getNode()->getType();
        for (const auto& lodMesh : type->getLodMeshes()) {
            auto rig = lodMesh.getMesh<mesh::Mesh>()->getRigContainer().get();
            if (!rig) continue;

            auto* clip = rig->m_clipContainer.findClip(m_clipName);
            if (clip) {
                m_clipIndex = clip->m_index;
                break;
            }
        }

        if (m_clipIndex < 0) {
            KI_WARN_OUT(fmt::format("CMD_ANIM_PLAY: MISSING_CLIP={}", m_clipName));
        }

        m_finished = m_clipIndex < 0;
    }

    void AnimationPlay::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* node = getNode();
        if (!node) return;

        auto& as = animation::AnimationSystem::get();

        as.startAnimation(
            m_handle,
            m_clipIndex,
            1.5f,
            m_speed,
            false,
            m_repeat,
            ctx.m_clock.ts);

        m_finished = true;
    }
}
