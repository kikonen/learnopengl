#pragma once

#include <vector>
#include <map>

#include <ode/ode.h>

#include "asset/Assets.h"

#include "Object.h"
#include "HeightMap.h"

class UpdateContext;

class Node;
class MeshType;

struct NodeInstance;


namespace physics {
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

        physics::physics_id registerObject();
        Object* getObject(physics::physics_id id);

        physics::height_map_id registerHeightMap();
        HeightMap* getHeightMap(physics::height_map_id id);

        float getWorldSurfaceLevel(const glm::vec3& pos);

    private:
        void preparePending(const UpdateContext& ctx);

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

        std::map<dGeomID, Object*> m_geomToObject;

    private:
        const Assets& m_assets;

        bool m_prepared{ false };

        glm::vec3 m_gravity{ 0, -9.81f, 0 };

        bool m_enabled{ false };
        float m_initialDelay{ 0.f };
        float m_remainder{ 0.f };

        size_t m_invokeCount{ 0 };
        size_t m_stepCount{ 0 };

        int m_staticPhysicsLevel{ -1 };
        int m_physicsLevel{ -1 };

        std::vector<physics::physics_id> m_pending;

        std::vector<Object*> m_updateObjects;
        std::vector<Object> m_objects;

        std::vector<HeightMap> m_heightMaps;
    };

}
