#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

#include <ode/ode.h>

#include "ki/size.h"

#include "Object.h"
#include "HeightMap.h"

struct UpdateContext;

namespace pool {
    struct NodeHandle;
}

namespace mesh {
    class MeshType;
    class Mesh;
}

struct NodeState;


namespace physics {
    class MeshGenerator;

    class PhysicsEngine {
        friend class MeshGenerator;

    public:
        static PhysicsEngine& get() noexcept;

        PhysicsEngine();
        PhysicsEngine& operator=(const PhysicsEngine&) = delete;

        ~PhysicsEngine();

        void prepare(std::shared_ptr<std::atomic<bool>> alive);
        void update(const UpdateContext& ctx);
        void updateBounds(const UpdateContext& ctx);

        inline bool isEnabled(bool enabled) const noexcept {
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

        uint32_t getObjectCount() const noexcept {
            return static_cast<uint32_t>(m_objects.size());
        }

        std::shared_ptr<std::vector<std::unique_ptr<mesh::Mesh>>> getObjectMeshes() const noexcept
        {
            return m_objectMeshes;
        }

        void generateObjectMeshes();

    private:
        void preparePending(const UpdateContext& ctx);
        void preparePendingNodes(const UpdateContext& ctx);

        void enforceBounds(
            const UpdateContext& ctx,
            const mesh::MeshType* type,
            Node& node,
            NodeState& state);

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

        std::shared_ptr<std::vector<std::unique_ptr<mesh::Mesh>>> m_objectMeshes;
    };
}
