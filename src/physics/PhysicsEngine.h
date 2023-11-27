#pragma once

#include <vector>

#include <ode/ode.h>

#include "asset/Assets.h"

class UpdateContext;

class Node;
class MeshType;

struct NodeInstance;


namespace physics {
    class Surface;

    class PhysicsEngine {
    public:
        PhysicsEngine(const Assets& assets);
        ~PhysicsEngine();

        void prepare();
        void update(const UpdateContext& ctx);

        Surface* registerSurface(std::unique_ptr<Surface> surface);
        float getWorldSurfaceLevel(const glm::vec3& pos);

    private:
        void enforceBounds(
            const UpdateContext& ctx,
            const MeshType& type,
            Node& node,
            NodeInstance& instance);

        void updateNode(
            const UpdateContext& ctx,
            const MeshType& type,
            Node& node,
            NodeInstance& instance);

    private:
        const Assets& m_assets;

        bool m_prepared{ false };

        dWorldID m_world{ nullptr };
        dSpaceID m_space{ nullptr };
        dJointGroupID m_contactgroup{ nullptr };

        int m_staticPhysicsLevel{ -1 };
        int m_physicsLevel{ -1 };

        std::vector<std::unique_ptr<Surface>> m_surfaces;
    };

}
