#include "PhysicsEngine.h"

#include <fmt/format.h>

#include "scene/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace physics {

    PhysicsEngine::PhysicsEngine(const Assets& assets)
        : m_assets(assets)
    {}

    void PhysicsEngine::prepare()
    {

    }

    void PhysicsEngine::update(const RenderContext& ctx)
    {
        for (const auto& all : ctx.m_registry->m_nodeRegistry->allNodes) {
            for (const auto& it : all.second) {
                const MeshType& type = *it.first.type;

                if (type.m_flags.enforceBounds) {
                    for (auto& node : it.second) {
                        enforceBounds(ctx, type, *node);
                    }
                }

                if (type.m_flags.physics) {
                    for (auto& node : it.second) {
                        updateNode(ctx, type, *node);
                    }
                }
            }
        }
    }

    void PhysicsEngine::enforceBounds(
        const RenderContext& ctx,
        const MeshType& type,
        Node& node)
    {
        const auto& worldPos = node.getWorldPosition();
        glm::vec3 pos = node.getPosition();

        auto surfaceY = getWorldSurfaceLevel(worldPos);

        auto* parent = ctx.m_registry->m_nodeRegistry->getParent(node);

        auto y = surfaceY - parent->getWorldPosition().y;
        y += node.getScale().y;
        pos.y = y;

        node.setPosition(pos);

        node.updateModelMatrix(parent);

        //KI_INFO_OUT(fmt::format("LEVEL: nodeId={}, level={}", node.m_objectID, level));
    }

    void PhysicsEngine::updateNode(
        const RenderContext& ctx,
        const MeshType& type,
        Node& node)
    {
    }

    Surface* PhysicsEngine::registerSurface(std::unique_ptr<Surface> surface)
    {
        m_surfaces.push_back(std::move(surface));
        return m_surfaces.back().get();
    }

    float PhysicsEngine::getWorldSurfaceLevel(const glm::vec3& pos)
    {
        float min = 0.f;
        bool hit = false;

        for (auto& surface : m_surfaces) {
            //if (!surface->withinBounds(pos)) continue;

            float level = surface->getLevel(pos);
            if (!hit || level < min) min = level;
            hit = true;
        }

        return hit ? min : pos.y;
    }
}

