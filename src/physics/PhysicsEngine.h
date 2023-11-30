#pragma once

#include <vector>
#include <map>

#include <ode/ode.h>

#include "asset/Assets.h"

#include "Object.h"

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

        inline bool isEnabled(bool enabled) const {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        void registerObject(Object* obj);

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

    public:
        dWorldID m_worldId{ nullptr };
        dSpaceID m_spaceId{ nullptr };
        dJointGroupID m_contactgroupId{ nullptr };

        std::map<dGeomID, Object*> m_objects;

    private:
        const Assets& m_assets;

        bool m_prepared{ false };

        glm::vec3 m_gravity{ 0, -9.81f, 0 };

        bool m_enabled{ false };
        float m_initialDelay{ 0.f };
        float m_remainder{ 0.f };

        int m_staticPhysicsLevel{ -1 };
        int m_physicsLevel{ -1 };

        std::vector<std::unique_ptr<Surface>> m_surfaces;
    };

}
