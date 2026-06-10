#pragma once

#include <functional>
#include <vector>
#include <cstdint>

#include "ki/size.h"

#include "render/size.h"

namespace model
{
    class Node;
    class NodeType;
}

class Program;

namespace mesh {
    struct LodMesh;
}

namespace render
{
    class RenderContext;
    struct DrawableInfo;

    // NOTE KI proramSelector != programPrrepare.
    // => selector is called from separate threads, prepare from RT only
    class CollectionRender
    {
    public:
        void drawProgram(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector,
            uint8_t kindBits,
            uint8_t routeBits)
        {
            drawNodesImpl(
                ctx,
                programSelector,
                [](ki::program_id programId) {},
                drawableSelector,
                kindBits,
                routeBits);
        }

        // NOTE KI special case render with prepare done for program
        // => not safe for generic render, since assumes that same prepare applies to
        //    all nodes consistently (if not then logic will fail), since prepare is done
        //    before all draw commands are executed
        void drawProgramWithPrepare(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector,
            uint8_t kindBits,
            uint8_t routeBits)
        {
            drawNodesImpl(ctx, programSelector, programPrepare, drawableSelector, kindBits, routeBits);
        }

        void drawBlendedImpl(
            const RenderContext& ctx,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector);

        // Shadow pass: sweeps ONLY the per-layer shadow-caster bucket (!noShadow), so a huge
        // noShadow set (e.g. a 1M-instance generator) isn't streamed-and-rejected per cascade.
        bool drawShadow(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector);

    private:
        bool drawNodesImpl(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector,
            uint8_t kindBits,
            uint8_t routeBits);

        // Emit one bucket of drawable indices (alive + visible + selected + has program).
        // For large buckets the filter + program resolution runs in parallel (selectors are
        // thread-safe per the class contract); programPrepare + batch emit stay serial on RT.
        // @return true if anything was emitted.
        bool sweepBucket(
            const RenderContext& ctx,
            const std::vector<uint32_t>& bucket,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector);

        // Scratch reused across sweepBucket calls: resolved program id per bucket slot (0 = skip).
        // Filled in parallel, drained serially. Lives on the (per-pass) instance, capacity reused.
        std::vector<ki::program_id> m_programScratch;
    };
}
