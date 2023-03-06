#include "PhysicsEngine.h"

#include <fmt/format.h>

#include "model/NodeInstance.h"

#include "scene/RenderContext.h"

#include "generator/NodeGenerator.h"

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
        for (const auto& all : ctx.m_registry->m_nodeRegistry->physicsNodes) {
            for (const auto& it : all.second) {
                const MeshType& type = *it.first.type;

                if (type.m_flags.physics) {
                    for (auto& node : it.second) {
                        if (node->m_instancer) {
                            for (auto& instance : node->m_instancer->getInstances()) {
                                updateNode(ctx, type, *node, instance);
                            }
                        }
                        else {
                            updateNode(ctx, type, *node, node->getInstance());
                        }
                    }
                }

                if (type.m_flags.enforceBounds) {
                    for (auto& node : it.second) {
                        if (node->m_instancer) {
                            for (auto& instance : node->m_instancer->getInstances()) {
                                enforceBounds(ctx, type, *node, instance);
                            }
                        }
                        else {
                            enforceBounds(ctx, type, *node, node->getInstance());
                        }
                    }
                }
            }
        }
    }

    void PhysicsEngine::enforceBounds(
        const RenderContext& ctx,
        const MeshType& type,
        Node& node,
        NodeInstance& instance)
    {
        int physicsLevel = type.m_flags.staticPhysics ? m_staticPhysicsLevel : m_physicsLevel;
        if (instance.m_physicsLevel == m_staticPhysicsLevel &&
            instance.m_matrixLevel == instance.m_physicsMatrixLevel) return;

        const auto& worldPos = instance.getWorldPosition();
        glm::vec3 pos = instance.getPosition();

        auto surfaceY = getWorldSurfaceLevel(worldPos);

        auto* parent = ctx.m_registry->m_nodeRegistry->getParent(node);

        auto y = surfaceY - parent->getWorldPosition().y;
        y += instance.getScale().y;
        pos.y = y;

        //KI_INFO_OUT(fmt::format(
        //    "({},{}, {}) => {}, {}, {}",
        //    worldPos.x, worldPos.z, worldPos.y, pos.x, pos.z, pos.y));

        instance.setPosition(pos);

        if (instance.m_dirty) {
            instance.updateModelMatrix(parent->getModelMatrix(), parent->getMatrixLevel());
        }

        instance.m_physicsMatrixLevel = instance.m_matrixLevel;
        instance.m_physicsLevel = physicsLevel;

        //KI_INFO_OUT(fmt::format("LEVEL: nodeId={}, level={}", node.m_objectID, level));
    }

    void PhysicsEngine::updateNode(
        const RenderContext& ctx,
        const MeshType& type,
        Node& node,
        NodeInstance& instance)
    {
    }

    Surface* PhysicsEngine::registerSurface(std::unique_ptr<Surface> surface)
    {
        m_surfaces.push_back(std::move(surface));
        m_physicsLevel++;
        m_staticPhysicsLevel++;
        return m_surfaces.back().get();
    }

    float PhysicsEngine::getWorldSurfaceLevel(const glm::vec3& pos)
    {
        float min = 0.f;
        bool hit = false;

        for (auto& surface : m_surfaces) {
            //if (!surface->withinBounds(pos)) continue;

            float level = surface->getLevel(pos);
            if (!hit || level > min) min = level;
            hit = true;
        }

        return hit ? min : 0.f;
    }
}

