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

    private:
        bool drawNodesImpl(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector,
            uint8_t kindBits,
            uint8_t routeBits);

        // Emit one bucket of drawable indices (alive + visible + selected + has program).
        // @return true if anything was emitted.
        bool sweepBucket(
            const RenderContext& ctx,
            const std::vector<uint32_t>& bucket,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const render::DrawableInfo&)>& drawableSelector);
    };
}
