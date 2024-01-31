#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include <ode/ode.h>

#include "ki/size.h"

#include "Object.h"
#include "HeightMap.h"

struct UpdateContext;

namespace pool {
    class NodeHandle;
}

namespace mesh {
    class MeshType;
}

struct NodeTransform;


namespace physics {
    class PhysicsEngine {
    public:
        PhysicsEngine(
            std::shared_ptr<std::atomic<bool>> alive);
        ~PhysicsEngine();

        void prepare();
        void update(const UpdateContext& ctx);
        void updateBounds(const UpdateContext& ctx);

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

        void handleNodeAdded(Node* node);

    private:
        void preparePending(const UpdateContext& ctx);
        void preparePendingNodes(const UpdateContext& ctx);

        void enforceBounds(
            const UpdateContext& ctx,
            const mesh::MeshType* type,
            Node& node,
            NodeTransform& transform);

    public:
        dWorldID m_worldId{ nullptr };
        dSpaceID m_spaceId{ nullptr };
        dJointGroupID m_contactgroupId{ nullptr };

        std::unordered_map<dGeomID, Object*> m_geomToObject;

    private:
        std::shared_ptr<std::atomic<bool>> m_alive;

        bool m_prepared{ false };

        glm::vec3 m_gravity{ 0, -9.81f, 0 };

        bool m_enabled{ false };
        float m_initialDelay{ 0.f };
        float m_remainder{ 0.f };

        size_t m_invokeCount{ 0 };
        size_t m_stepCount{ 0 };

        std::vector<pool::NodeHandle> m_enforceBoundsDynamic;

        std::vector<pool::NodeHandle> m_enforceBoundsStatic;

        std::vector<pool::NodeHandle> m_pendingNodes;

        std::vector<physics::physics_id> m_pending;

        std::vector<Object*> m_updateObjects;
        std::vector<Object> m_objects;

        std::vector<HeightMap> m_heightMaps;

    };
}
