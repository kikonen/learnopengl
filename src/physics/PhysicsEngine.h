#pragma once

#include <vector>

#include "asset/Assets.h"

#include "Surface.h"

class RenderContext;
class Node;
class MeshType;

namespace physics {
    class PhysicsEngine {
    public:
        PhysicsEngine(const Assets& assets);

        void prepare();
        void update(const RenderContext& ctx);

        Surface* registerSurface(std::unique_ptr<Surface> surface);
        float getWorldSurfaceLevel(const glm::vec3& pos);

    private:
        void enforceBounds(
            const RenderContext& ctx,
            const MeshType& type,
            Node& node);

        void updateNode(
            const RenderContext& ctx,
            const MeshType& type,
            Node& node);

    private:
        const Assets& m_assets;

        std::vector<std::unique_ptr<Surface>> m_surfaces;
    };

}
