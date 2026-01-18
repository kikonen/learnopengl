#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <tuple>
#include <span>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "Object.h"
#include "HeightMap.h"

struct UpdateContext;

namespace model
{
    class NodeType;
}

namespace JPH {
    class PhysicsSystem;
    class BodyInterface;
}

namespace physics {
    struct RayHit;
    class MeshGenerator;
    class JoltFoundation;

    class PhysicsSystem {
        friend class MeshGenerator;

    public:
        static void init() noexcept;
        static void release() noexcept;
        static PhysicsSystem& get() noexcept;

        PhysicsSystem();
        PhysicsSystem& operator=(const PhysicsSystem&) = delete;

        ~PhysicsSystem();

        void clear();
        void prepare(
            const std::shared_ptr<std::atomic_bool>& alive);

        void updatePrepare(const UpdateContext& ctx);
        void updateObjects(const UpdateContext& ctx);

        inline bool isEnabled() const noexcept {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        ki::level_id getLevel() const noexcept
        {
            return m_level;
        }

        physics::object_id registerObject(
            pool::NodeHandle nodeHandle,
            uint32_t entityIndex,
            bool update,
            physics::Object object);

        void unregisterObject(
            physics::object_id objectId);

        const physics::Object* getObject(physics::object_id id) const;
        const pool::NodeHandle& getNodeHandle(physics::object_id id) const;

        physics::height_map_id registerHeightMap();
        const HeightMap* getHeightMap(physics::height_map_id id) const;
        HeightMap* modifyHeightMap(physics::height_map_id id);

        // Register shape into world
        void registerShape(
            physics::Shape& shape,
            const glm::vec3& scale);

        std::pair<bool, float> getWorldSurfaceLevel(
            const glm::vec3& pos,
            const glm::vec3 dir,
            uint32_t collisionMask) const;

        std::vector<std::pair<bool, float>> getWorldSurfaceLevels(
            std::span<glm::vec3> positions,
            const glm::vec3 dir,
            uint32_t collisionMask) const;

        uint32_t getObjectCount() const noexcept {
            return static_cast<uint32_t>(m_objects.size());
        }

        physics::RayHit rayCastClosest(
            const glm::vec3& origin,
            const glm::vec3& dir,
            float distance,
            uint32_t collisionMask,
            pool::NodeHandle fromNode) const;

        std::vector<physics::RayHit> rayCastClosestToMultiple(
            const glm::vec3& origin,
            const std::vector<glm::vec3>& dirs,
            float distance,
            uint32_t collisionMask,
            pool::NodeHandle fromNode) const;

        std::vector<physics::RayHit> rayCastClosestFromMultiple(
            std::span<glm::vec3> origin,
            const glm::vec3& dir,
            float distance,
            uint32_t collisionMask,
            pool::NodeHandle fromNode) const;

        bool hasPending() const noexcept
        {
            return !m_pending.empty();
        }

        // Access to Jolt systems
        JPH::PhysicsSystem& getJoltPhysicsSystem();
        const JPH::PhysicsSystem& getJoltPhysicsSystem() const;
        JPH::BodyInterface& getBodyInterface();
        const JPH::BodyInterface& getBodyInterface() const;

    private:
        void preparePending(const UpdateContext& ctx);

        void generateObjectMeshes();

    private:
        std::shared_ptr<std::atomic_bool> m_alive;

        mutable std::mutex m_lock;

        bool m_prepared{ false };

        glm::vec3 m_gravity{ 0, -9.81f, 0 };

        bool m_enabled{ false };
        float m_initialDelay{ 0.f };
        float m_elapsedTotal{ 0.f };
        float m_remainder{ 0.f };

        size_t m_invokeCount{ 0 };
        size_t m_stepCount{ 0 };

        // INDEX = pending index
        std::vector<physics::object_id> m_pending;

        // Free deleted slots
        // => reuse slots after RT is synced with WT
        std::vector<physics::object_id> m_freeIndeces;

        // INDEX = objectId
        std::vector<pool::NodeHandle> m_nodeHandles;
        // INDEX = objectId
        std::vector<uint32_t> m_entityIndeces;
        // INDEX = objectId
        std::vector<physics::Object> m_objects;

        ki::level_id m_level{ 0 };
        // INDEX = objectId
        std::vector<ki::level_id> m_matrixLevels;
        // INDEX = objectId
        std::vector<physics::object_id> m_updateObjects;

        std::vector<HeightMap> m_heightMaps;

        // Map from Jolt BodyID to object_id
        std::unordered_map<uint32_t, physics::object_id> m_bodyIdToObject;

        std::unordered_map<pool::NodeHandle, physics::object_id> m_handleToId;

        std::unique_ptr<physics::MeshGenerator> m_meshGenerator;
    };
}
