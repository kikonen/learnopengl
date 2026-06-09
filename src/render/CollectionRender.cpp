#include "CollectionRender.h"

#include <vector>
#include <utility>
#include <algorithm>

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

        // NOTE KI a BLEND drawable is always also ALPHA (spec), so it lives in BOTH the alpha
        // and blended buckets. To avoid emitting it twice in a pass that requests both axes
        // (e.g. KIND_ALL g-buffer/shadow): sweep blended ONLY when blend is requested without
        // alpha (the OIT pass). Alpha-including passes cover ALPHA|BLEND via the alpha bucket.
        // (Relies on the kind model SOLID | ALPHA | ALPHA|BLEND — see NodeCollection::addDrawables.)
        if (kindBits & render::KIND_SOLID) sweep(layerDrawables.solid);
        if (kindBits & render::KIND_ALPHA) sweep(layerDrawables.alpha);
        // blend-only (OIT) pass: only the OIT blend sub-bucket; forward "effect" blend goes
        // through drawBlendedImpl (depth-sorted). alpha-including passes cover the alpha-tested
        // part of blend drawables via the alpha bucket.
        if (kindBits & render::KIND_BLEND && !(kindBits & render::KIND_ALPHA)) sweep(layerDrawables.blendOit);

        return rendered;
    }

    // Forward-rendered blend ("effect") pass: depth-sorted back-to-front. This is NOT the OIT
    // blend path (OIT goes through the KIND_BLEND sweep in drawNodesImpl over blendOit); kept
    // separate because it needs per-drawable distance sorting. Iterates only the forward blend
    // sub-bucket so OIT drawables aren't redundantly scanned.
    void CollectionRender::drawBlendedImpl(
        const RenderContext& ctx,
        const std::function<bool(const render::DrawableInfo&)>& drawableSelector)
    {
        auto& collection = *ctx.m_collection;
        if (ctx.m_layer >= MAX_LAYERS) return;

        const auto& bucket = collection.m_drawablesByLayer[ctx.m_layer].blendForward;
        if (bucket.empty()) return;

        const auto drawables = InstanceRegistry::get().getDrawables();
        const glm::vec3& eyePos = ctx.m_camera->getWorldPosition();

        // collect visible, selected forward-blend drawables with their squared eye distance
        std::vector<std::pair<float, uint32_t>> sorted;
        sorted.reserve(bucket.size());
        for (const uint32_t index : bucket) {
            const auto& drawable = drawables[index];
            if (drawable.entityIndex == 0) continue;
            if ((drawable.m_visibility & VISIBLE_ALL) != VISIBLE_ALL) continue;
            if (!drawableSelector(drawable)) continue;

            const glm::vec3 delta = drawable.worldVolume.getCenter() - eyePos;
            sorted.emplace_back(glm::dot(delta, delta), index);
        }

        if (sorted.empty()) return;

        // NOTE KI blending is *NOT* optimal program / nodetype wise due to depth sorting.
        // order = from furthest away to nearest (back-to-front). Per-drawable sort (finer
        // than the old per-node sort) and no same-distance discard (vector, not map).
        if (sorted.size() > 1) {
            std::sort(
                sorted.begin(), sorted.end(),
                [](const auto& a, const auto& b) { return a.first > b.first; });
        }

        for (const auto& [dist2, index] : sorted) {
            const auto& drawable = drawables[index];

            const auto programId = drawable.programId;
            if (!programId) continue;

            ctx.m_batch->addDrawable(index, drawable, programId);
        }
    }
}
