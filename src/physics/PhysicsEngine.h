#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>

#include <ode/ode.h>

#include "ki/size.h"

#include "Object.h"
#include "NodeBounds.h"
#include "HeightMap.h"

struct UpdateContext;

namespace pool {
    class NodeHandle;
}

namespace mesh {
    class MeshType;
}

struct NodeTransform;
struct Snapshot;
class Registry;
class NodeSnapshotRegistry;

namespace physics {
    class ObjectSnapshotRegistry;

    class PhysicsEngine {
    public:
        static PhysicsEngine& get() noexcept;

        PhysicsEngine();
        PhysicsEngine& operator=(const PhysicsEngine&) = delete;

        ~PhysicsEngine();

        void prepare(
            std::shared_ptr<std::atomic<bool>> alive,
            Registry* registry);

        void updateWT(const UpdateContext& ctx);
        void updateBounds(const UpdateContext& ctx);

        void updatePendingSnapshots();
        void updateActiveSnapshots();

        inline bool isEnabled() const noexcept{
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        std::tuple<physics::physics_id, uint32_t> registerObject(
            const physics::Object& obj);

        physics::height_map_id registerHeightMap();
        HeightMap* getHeightMap(physics::height_map_id id);

        float getWorldSurfaceLevel(const glm::vec3& pos);

        void registerBoundsNode(Node* node);

        uint32_t getObjectCount() const noexcept {
            return static_cast<uint32_t>(m_objects.size());
        }

        uint32_t getStaticBoundsCount() const noexcept {
            return static_cast<uint32_t>(m_staticBounds.size());
        }

        uint32_t getDynamicBoundsCount() const noexcept {
            return static_cast<uint32_t>(m_dynamicBounds.size());
        }

    private:
        void preparePendingObjects(const UpdateContext& ctx);
        void preparePendingBounds(const UpdateContext& ctx);

        void enforceBounds(
            const UpdateContext& ctx,
            NodeBounds& bounds,
            const mesh::MeshType* type,
            Node& node,
            const Snapshot& snapshot);

    public:
        dWorldID m_worldId{ nullptr };
        dSpaceID m_spaceId{ nullptr };
        dJointGroupID m_contactgroupId{ nullptr };

        std::unordered_map<dGeomID, Object*> m_geomToObject;

    private:
        std::shared_ptr<std::atomic<bool>> m_alive;

        Registry* m_registry{ nullptr };

        ObjectSnapshotRegistry* m_workerObjectSnapshotRegistry;
        ObjectSnapshotRegistry* m_pendingObjectSnapshotRegistry;
        ObjectSnapshotRegistry* m_activeObjectSnapshotRegistry;

        NodeSnapshotRegistry* m_pendingSnapshotRegistry;

        std::mutex m_lock;
        std::mutex m_pendingLock;

        bool m_prepared{ false };

        glm::vec3 m_gravity{ 0, -9.81f, 0 };

        bool m_enabled{ false };
        float m_initialDelay{ 0.f };
        float m_remainder{ 0.f };

        size_t m_invokeCount{ 0 };
        size_t m_stepCount{ 0 };

        std::vector<physics::NodeBounds> m_dynamicBounds;

        std::vector<physics::NodeBounds> m_staticBounds;

        std::vector<physics::NodeBounds> m_pendingNodes;

        std::vector<physics::physics_id> m_pendingObjects;

        std::vector<Object*> m_updateObjects;
        std::vector<Object> m_objects;

        std::vector<HeightMap> m_heightMaps;

    };
}
