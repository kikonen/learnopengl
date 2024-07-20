#include "AnimationPlay.h"

#include "ki/limits.h"

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
        ki::node_id nodeId,
        std::string clipName,
        float speed,
        bool repeat) noexcept
        : NodeCommand(nodeId, 0, false),
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
        m_finished = m_clipIndex < 0;
    }

    void AnimationPlay::execute(
        const UpdateContext& ctx) noexcept
    {
        auto* node = getNode();
        if (!node) return;

        auto& as = animation::AnimationSystem::get();

        as.startAnimation(
            m_nodeHandle,
            m_clipIndex,
            1.5f,
            m_speed,
            false,
            m_repeat,
            ctx.m_clock.ts);

        m_finished = true;
    }
}
