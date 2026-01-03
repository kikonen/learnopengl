#pragma once

#include <functional>

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
            const std::function<bool(const model::Node*)>& nodeSelector,
            uint8_t kindBits)
        {
            drawNodesImpl(
                ctx,
                programSelector,
                [](ki::program_id programId) {},
                nodeSelector,
                kindBits);
        }

        // NOTE KI special case render with prepare done for program
        // => not safe for generic render, since assumes that same prepare applies to
        //    all nodes consistently (if not then logic will fail), since prepare is done
        //    before all draw commands are executed
        void drawProgramWithPrepare(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const model::Node*)>& nodeSelector,
            uint8_t kindBits)
        {
            drawNodesImpl(ctx, programSelector, programPrepare, nodeSelector, kindBits);
        }

        void drawBlendedImpl(
            const RenderContext& ctx,
            const std::function<bool(const model::Node*)>& nodeSelector);

    private:
        bool drawNodesImpl(
            const RenderContext& ctx,
            const std::function<ki::program_id(const render::DrawableInfo&)>& programSelector,
            const std::function<void(ki::program_id)>& programPrepare,
            const std::function<bool(const model::Node*)>& nodeSelector,
            uint8_t kindBits);
    };
}
