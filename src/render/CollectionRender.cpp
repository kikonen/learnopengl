#include "CollectionRender.h"

#include "kigl/GLState.h"

#include "model/Node.h"

#include "debug/DebugContext.h"
#include "render/RenderContext.h"
#include "render/FrameBuffer.h"
#include "render/Batch.h"
#include "render/DrawableInfo.h"
#include "render/InstanceRegistry.h"
#include "render/Camera.h"
#include "render/NodeCollection.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace render
{
    bool CollectionRender::drawNodesImpl(
        const RenderContext& ctx,
        const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
        const std::function<void(ki::program_id)>& programPrepare,
        const std::function<bool(const render::DrawableInfo&)>& drawableSelector,
        const uint8_t kindBits)
    {
        bool rendered{ false };

        auto& collection = *ctx.m_collection;
        if (ctx.m_layer >= MAX_LAYERS) return false;

        const auto drawables = InstanceRegistry::get().getDrawables();
        const auto& layerDrawables = collection.m_drawablesByLayer[ctx.m_layer];

        // Per-drawable sweep over this layer's buckets. Node-level checks are all expressed
        // per drawable now: alive => entityIndex!=0 (+removed from bucket on node removal),
        // visible+frustum+LOD => m_visibility (set by cullFrustum), layer => bucket key,
        // type-invisible => never bucketed.
        const auto sweep = [&](const std::vector<uint32_t>& bucket)
            {
                for (const uint32_t index : bucket) {
                    const auto& drawable = drawables[index];
                    if (drawable.entityIndex == 0) continue;
                    if ((drawable.m_visibility & VISIBLE_ALL) != VISIBLE_ALL) continue;
                    if (!drawableSelector(drawable)) continue;

                    const auto programId = programSelector(drawable);
                    if (!programId) continue;

                    programPrepare(programId);
                    ctx.m_batch->addDrawable(index, drawable, programId);

                    rendered = true;
                }
            };

        if (kindBits & render::KIND_SOLID) sweep(layerDrawables.solid);
        if (kindBits & render::KIND_ALPHA) sweep(layerDrawables.alpha);
        if (kindBits & render::KIND_BLEND) sweep(layerDrawables.blended);

        return rendered;
    }

    void CollectionRender::drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const render::DrawableInfo&)>& drawableSelector)
    {
        auto& collection = *ctx.m_collection;

        if (collection.m_blendedNodes.empty()) return;

        const glm::vec3& eyePos = ctx.m_camera->getWorldPosition();

        // TODO KI discards nodes if *same* distance
        std::map<float, model::Node*> sorted;
        for (const auto& handle : collection.m_blendedNodes) {
            auto* node = handle.toNode();
            if (!node) continue;
            if (!node->m_alive) continue;
            if (node->m_layer != ctx.m_layer) continue;

            const auto* snapshot = node->getSnapshotRT();
            if (!snapshot) continue;

            const auto& pos = snapshot->getWorldPosition();
            const float dist2 = glm::distance2(eyePos, pos);
            sorted[dist2] = node;
        }

        if (!sorted.empty()) {
            //glMemoryBarrier(GL_ALL_BARRIER_BITS);
            glFlush();
            //glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT | GL_FRAMEBUFFER_BARRIER_BIT);
            //glFinish();
        }

        // NOTE KI blending is *NOT* optimal program / nodetypw wise due to depth sorting
        // NOTE KI order = from furthest away to nearest
        for (std::map<float, model::Node*>::reverse_iterator it = sorted.rbegin(); it != sorted.rend(); ++it) {
            auto* node = it->second;
            if (!node->m_alive) continue;
            if (node->m_typeFlags.invisible || !node->m_visible) continue;

            node->addToBatch(
                ctx,
                [this](const render::DrawableInfo& drawable) { return drawable.programId; },
                [](ki::program_id) {},
                drawableSelector,
                render::KIND_BLEND,
                *ctx.m_batch);
        }

        // TODO KI if no flush here then render order of blended nodes is incorrect
        //ctx.m_batch->flush(ctx);
    }
}
