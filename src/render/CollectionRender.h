#pragma once

#include <functional>

#include "ki/size.h"

#include "render/size.h"

class RenderContext;

class Program;
class Node;
class NodeType;

namespace mesh {
    struct LodMesh;
}

namespace render
{
    class CollectionRender
    {
    public:
        void drawProgram(
            const RenderContext& ctx,
            const std::function<ki::program_id(const mesh::LodMesh&)>& programSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits);

        bool drawNodesImpl(
            const RenderContext& ctx,
            const std::function<ki::program_id(const mesh::LodMesh&)>& programSelector,
            const std::function<bool(const Node*)>& nodeSelector,
            uint8_t kindBits);

        void drawBlendedImpl(
            const RenderContext& ctx,
            const std::function<bool(const Node*)>& nodeSelector);
    };
}
