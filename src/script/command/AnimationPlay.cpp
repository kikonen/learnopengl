#include "AnimationPlay.h"

#include "ki/limits.h"

#include "util/debug.h"

#include "model/Node.h"

#include "animation/Rig.h"
#include "animation/AnimationSystem.h"

#include "mesh/Mesh.h"

#include "engine/UpdateContext.h"

#include "audio/AudioSystem.h"

#include "registry/Registry.h"

namespace script
{
    AnimationPlay::AnimationPlay(
        pool::NodeHandle handle,
        ki::sid_t clipId,
        float speed,
        bool repeat) noexcept
        : NodeCommand(handle, 0, false),
        m_clipId{ clipId },
        m_speed{ speed },
        m_repeat{ repeat }
    {
    }

    void AnimationPlay::bind(const UpdateContext& ctx) noexcept
    {
        NodeCommand::bind(ctx);

        auto* node = getNode();
        if (!node) return;

        std::set<animation::Rig*> processedRigs;
        bool skipped = false;

        for (const auto& lodMesh : node->getLodMeshes()) {
            const auto* mesh = lodMesh.getMesh<mesh::Mesh>();

            auto rig = mesh->getRig();
            if (!rig) continue;

            if (!lodMesh.m_flags.useAnimation) {
                KI_INFO(fmt::format(
                    "CMD::ANIM_PLAY: SKIP_DISABLED: mesh={}, sid={}, name={}",
                    mesh->m_name, m_clipId, SID_NAME(m_clipId)));
                skipped = true;
                continue;
            }

            if (processedRigs.contains(rig)) continue;
            processedRigs.insert(rig);

            auto* clip = rig->getClipContainer().findClip(m_clipId);
            if (clip) {
                m_clipIndex = clip->m_index;
                break;
            }
        }

        if (!skipped && m_clipIndex < 0) {
            KI_WARN_OUT(fmt::format("CMD::ANIM_PLAY: MISSING_CLIP: sid={}, name={}", m_clipId, SID_NAME(m_clipId)));
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
            ctx.getClock().ts);

        m_finished = true;
    }
}
