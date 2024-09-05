#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>
#include <tuple>
#include <span>

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
}


namespace physics {
    struct RayHit;
    class MeshGenerator;

    class PhysicsEngine {
        friend class MeshGenerator;

    public:
        static PhysicsEngine& get() noexcept;

        PhysicsEngine();
        PhysicsEngine& operator=(const PhysicsEngine&) = delete;

        ~PhysicsEngine();

        void prepare(std::shared_ptr<std::atomic<bool>> alive);

        void updatePrepare(const UpdateContext& ctx);
        void updateObjects(const UpdateContext& ctx);

        inline bool isEnabled() const noexcept {
            return m_enabled;
        }

        inline void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        physics::physics_id registerObject();
        Object* getObject(physics::physics_id id);

        physics::height_map_id registerHeightMap();
        const HeightMap* getHeightMap(physics::height_map_id id) const;
        HeightMap* modifyHeightMap(physics::height_map_id id);

        dGeomID addGeom(const physics::Geom& geom);
        void removeGeom(dGeomID physicId);

        std::pair<bool, float> getWorldSurfaceLevel(
            const glm::vec3& pos,
            uint32_t categoryMask,
            uint32_t collisionMask);

        std::vector<std::pair<bool, float>> getWorldSurfaceLevels(
            std::span<glm::vec3> positions,
            const glm::vec3 dir,
            uint32_t collisionMask);

        uint32_t getObjectCount() const noexcept {
            return static_cast<uint32_t>(m_objects.size());
        }

        void generateObjectMeshes();

        std::vector<physics::RayHit> rayCast(
            const glm::vec3& origin,
            const glm::vec3& dir,
            float distance,
            uint32_t categoryMask,
            uint32_t collisionMask,
            pool::NodeHandle fromNode,
            bool onlyClosest);

        std::vector<std::pair<bool, physics::RayHit>> rayCast(
            std::span<glm::vec3> origin,
            const glm::vec3& dir,
            float distance,
            uint32_t categoryMask,
            uint32_t collisionMask,
            pool::NodeHandle fromNode);

    private:
        void preparePending(const UpdateContext& ctx);

    public:
        dWorldID m_worldId{ nullptr };
        dSpaceID m_spaceId{ nullptr };
        dJointGroupID m_contactgroupId{ nullptr };

        std::unordered_map<dGeomID, Object*> m_geomToObject;

    private:
        std::shared_ptr<std::atomic<bool>> m_alive;

        std::mutex m_lock;

        bool m_prepared{ false };

        glm::vec3 m_gravity{ 0, -9.81f, 0 };

        bool m_enabled{ false };
        float m_initialDelay{ 0.f };
        float m_elapsedTotal{ 0.f };
        float m_remainder{ 0.f };

        size_t m_invokeCount{ 0 };
        size_t m_stepCount{ 0 };

        std::vector<physics::physics_id> m_pending;

        std::vector<Object*> m_updateObjects;
        std::vector<Object> m_objects;
        physics::physics_id m_rayId{ 0 };

        std::vector<HeightMap> m_heightMaps;

        std::unique_ptr<physics::MeshGenerator> m_meshGenerator;
    };
}
