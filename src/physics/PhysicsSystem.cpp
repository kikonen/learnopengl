#include "PhysicsSystem.h"

#include <iostream>

#include <fmt/format.h>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseQuery.h>

#include "asset/Assets.h"

#include "util/thread.h"
#include "util/glm_format.h"
#include "util/util.h"

#include "model/Node.h"
#include "model/NodeState.h"

#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"

#include "debug/DebugContext.h"

#include "engine/UpdateContext.h"

#include "generator/NodeGenerator.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "Surface.h"

#include "physics/MeshGenerator.h"
#include "physics/RayHit.h"
#include "physics/physics_util.h"
#include "physics/jolt_util.h"
#include "physics/JoltFoundation.h"

namespace {
    constexpr float STEP_SIZE = 0.03f;
    constexpr int COLLISION_STEPS = 1;

    size_t debugCounter{ 0 };

    std::shared_ptr<std::vector<mesh::MeshInstance>> NO_MESHES;

    static physics::PhysicsSystem* s_system{ nullptr };

    // Custom body filter that checks collision masks stored in user data
    class CollisionMaskBodyFilter : public JPH::BodyFilter {
    public:
        CollisionMaskBodyFilter(uint32_t collisionMask, JPH::BodyID excludeBody = JPH::BodyID())
            : m_collisionMask(collisionMask)
            , m_excludeBody(excludeBody)
        {}

        virtual bool ShouldCollide(const JPH::BodyID& inBodyID) const override {
            if (inBodyID == m_excludeBody) return false;
            return true;
        }

        virtual bool ShouldCollideLocked(const JPH::Body& inBody) const override {
            if (inBody.GetID() == m_excludeBody) return false;

            // Check collision mask from user data
            uint64_t userData = inBody.GetUserData();
            auto bodyCategory = physics::unpackCategory(userData);

            // Body should collide if its category matches our collision mask
            return (mask(bodyCategory) & m_collisionMask) != 0;
        }

    private:
        uint32_t m_collisionMask;
        JPH::BodyID m_excludeBody;
    };
}

namespace physics
{
    void PhysicsSystem::init() noexcept
    {
        assert(!s_system);

        // Initialize Jolt foundation first
        JoltFoundation::init();

        s_system = new PhysicsSystem();
    }

    void PhysicsSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;

        // Release Jolt foundation
        JoltFoundation::release();
    }

    PhysicsSystem& PhysicsSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }
}

namespace physics
{
    PhysicsSystem::PhysicsSystem()
    {
        m_heightMaps.emplace_back();
        registerObject({}, 0, false, {});
    }

    PhysicsSystem::~PhysicsSystem()
    {
        // Clear objects first (releases Jolt bodies)
        clear();
    }

    void PhysicsSystem::clear()
    {
        ASSERT_RT();

        // Release all physics objects
        auto& bodyInterface = getBodyInterface();
        for (auto& obj : m_objects) {
            obj.release(bodyInterface);
        }

        m_elapsedTotal = 0.f;
        m_remainder = 0.f;

        m_invokeCount = 0;
        m_stepCount = 0;

        m_pending.clear();

        m_freeIndeces.clear();

        m_nodeHandles.clear();
        m_entityIndeces.clear();
        m_objects.clear();

        m_level = 0;
        m_matrixLevels.clear();
        m_updateObjects.clear();

        m_heightMaps.clear();
        m_bodyIdToObject.clear();
        m_handleToId.clear();

        {
            // NOTE KI register NULL object
            registerObject({}, 0, false, {});
            m_heightMaps.emplace_back();
        }

        {
            auto& dbg = debug::DebugContext::modify();
            auto& physicsDbg = dbg.m_physics;

            if (m_meshGenerator) {
                m_meshGenerator->clear();
            }

            std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
            physicsDbg.m_meshesWT.store(tmp);
            physicsDbg.m_meshesPending.store(tmp);
            physicsDbg.m_meshesRT.store(tmp);
        }
    }

    void PhysicsSystem::prepare(
        const std::shared_ptr<std::atomic_bool>& alive)
    {
        m_prepared = true;
        m_alive = alive;

        const auto& assets = Assets::get();

        m_elapsedTotal = 0.f;
        m_initialDelay = assets.physicsInitialDelay;

        // Prepare Jolt foundation
        JoltFoundation::get().prepare();

        // Set gravity
        m_gravity = { 0, -2.01f, 0 };
        getJoltPhysicsSystem().SetGravity(toJolt(m_gravity));

        m_meshGenerator = std::make_unique<physics::MeshGenerator>(*this);
    }

    void PhysicsSystem::updatePrepare(const UpdateContext& ctx)
    {
        ASSERT_WT();

        if (!m_enabled) return;

        preparePending(ctx);
    }

    void PhysicsSystem::updateObjects(const UpdateContext& ctx)
    {
        ASSERT_WT();

        if (!m_enabled) return;

        m_elapsedTotal += ctx.getClock().elapsedSecs;
        if (m_elapsedTotal < m_initialDelay) return;

        auto& dbg = debug::DebugContext::modify();
        auto& physicsDbg = dbg.m_physics;

        auto& nodeRegistry = NodeRegistry::get();
        auto& bodyInterface = getBodyInterface();

        bool updatedPhysics = false;
        for (const auto id : m_updateObjects) {
            if (!id) continue;
            auto& obj = m_objects[id];
            updatedPhysics |= obj.updateToPhysics(m_entityIndeces[id], m_matrixLevels[id], bodyInterface, nodeRegistry);
        }

        const float dtTotal = ctx.getClock().elapsedSecs + m_remainder;
        const int steps = static_cast<int>(dtTotal / STEP_SIZE);
        m_remainder = dtTotal - steps * STEP_SIZE;

        if (steps > 0)
        {
            m_invokeCount++;
            m_stepCount += steps;

            if (physicsDbg.m_updateEnabled)
            {
                auto& joltFoundation = JoltFoundation::get();

                for (int i = 0; i < steps; i++) {
                    if (!*m_alive) return;

                    // Update physics simulation
                    getJoltPhysicsSystem().Update(
                        STEP_SIZE,
                        COLLISION_STEPS,
                        joltFoundation.getTempAllocator(),
                        joltFoundation.getJobSystem());
                }

                for (size_t i = 0; i < m_objects.size(); i++) {
                    auto& obj = m_objects[i];
                    obj.updateFromPhysics(m_entityIndeces[i], bodyInterface, nodeRegistry);
                }
            }

            generateObjectMeshes();
        }
        else {
            if (updatedPhysics) {
                generateObjectMeshes();
            }
        }
    }

    void PhysicsSystem::preparePending(const UpdateContext& ctx)
    {
        ASSERT_WT();

        if (m_pending.empty()) return;

        auto& nodeRegistry = NodeRegistry::get();
        auto& joltPhysicsSystem = getJoltPhysicsSystem();
        auto& bodyInterface = getBodyInterface();

        std::unordered_map<physics::object_id, bool> prepared;

        for (const auto& id : m_pending) {
            auto& obj = m_objects[id];
            auto* node = m_nodeHandles[id].toNode();

            if (node) {
                auto entityIndex = m_entityIndeces[id];
                obj.create(id, entityIndex, joltPhysicsSystem, nodeRegistry);

                // Map body ID to object ID
                if (obj.m_body.hasPhysicsBody()) {
                    m_bodyIdToObject.insert({ obj.m_body.m_bodyId.GetIndex(), id });
                }
                if (obj.m_shape.hasPhysicsBody()) {
                    m_bodyIdToObject.insert({ obj.m_shape.m_staticBodyId.GetIndex(), id });
                }

                // Handle heightmaps
                for (auto& heightMap : m_heightMaps) {
                    if (heightMap.m_origin == node) {
                        heightMap.create(joltPhysicsSystem, obj);
                    }
                }

                obj.updateToPhysics(entityIndex, m_matrixLevels[id], bodyInterface, nodeRegistry);

                m_handleToId.insert({ m_nodeHandles[id], id });
            }
            else {
                obj.create(id, 0, joltPhysicsSystem, nodeRegistry);
            }

            m_level++;

            prepared.insert({ id, true });
        }

        if (!prepared.empty()) {
            const auto& it = std::remove_if(
                m_pending.begin(),
                m_pending.end(),
                [&prepared](auto& id) {
                    return prepared.find(id) != prepared.end();
                });
            m_pending.erase(it, m_pending.end());
        }
    }

    physics::object_id PhysicsSystem::registerObject(
        pool::NodeHandle nodeHandle,
        uint32_t entityIndex,
        bool update,
        physics::Object src)
    {
        if (entityIndex > 0) {
            ASSERT_WT();
        }

        auto id = static_cast<physics::object_id>(m_objects.size());

        m_objects.push_back(std::move(src));

        m_nodeHandles.push_back(nodeHandle);
        m_entityIndeces.push_back(entityIndex);
        // NOTE KI use max value to ensure first updateToPhysics sync always runs
        // (node's matrixLevel starts at 0, so 0 == 0 would skip the initial sync)
        m_matrixLevels.push_back(static_cast<ki::level_id>(0));
        m_updateObjects.push_back(update ? id : 0);

        m_pending.push_back(id);

        return id;
    }

    void PhysicsSystem::unregisterObject(
        physics::object_id objectId)
    {
        if (objectId < 1 || objectId > m_objects.size()) return;

        const auto objectIndex = objectId;

        auto& obj = m_objects[objectIndex];

        // Remove body mappings
        if (obj.m_body.hasPhysicsBody()) {
            m_bodyIdToObject.erase(obj.m_body.m_bodyId.GetIndex());
        }
        if (obj.m_shape.hasPhysicsBody()) {
            m_bodyIdToObject.erase(obj.m_shape.m_staticBodyId.GetIndex());
        }

        // Release physics objects
        obj.release(getBodyInterface());

        // Clear slot
        m_objects[objectIndex] = std::move(physics::Object{});

        m_nodeHandles[objectIndex] = pool::NodeHandle::NULL_HANDLE;
        m_entityIndeces[objectIndex] = 0;
        m_matrixLevels[objectIndex] = 0;
        m_updateObjects[objectIndex] = 0;

        m_freeIndeces.push_back(objectIndex);

        // Discard possible stale references
        {
            const auto& it = std::remove_if(
                m_pending.begin(),
                m_pending.end(),
                [objectId](auto& id) {
                    return id == objectId;
                });
            m_pending.erase(it, m_pending.end());
        }
    }

    const Object* PhysicsSystem::getObject(physics::object_id id) const
    {
        ASSERT_WT();

        if (id < 1 || id > m_objects.size()) return nullptr;
        return &m_objects[id];
    }

    const pool::NodeHandle& PhysicsSystem::getNodeHandle(physics::object_id id) const
    {
        ASSERT_WT();

        if (id < 1 || id > m_nodeHandles.size()) return pool::NodeHandle::NULL_HANDLE;
        return m_nodeHandles[id];
    }

    physics::height_map_id PhysicsSystem::registerHeightMap()
    {
        ASSERT_WT();

        auto& map = m_heightMaps.emplace_back<HeightMap>({});
        map.m_id = static_cast<physics::height_map_id>(m_heightMaps.size() - 1);

        return map.m_id;
    }

    const HeightMap* PhysicsSystem::getHeightMap(physics::height_map_id id) const
    {
        ASSERT_WT();

        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id];
    }

    HeightMap* PhysicsSystem::modifyHeightMap(physics::height_map_id id)
    {
        ASSERT_WT();

        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id];
    }

    void PhysicsSystem::registerShape(
        physics::Shape& shape,
        const glm::vec3& scale)
    {
        ASSERT_WT();

        shape.create(
            0,
            getJoltPhysicsSystem(),
            scale,
            JPH::BodyID());
    }

    std::pair<bool, float> PhysicsSystem::getWorldSurfaceLevel(
        const glm::vec3& pos,
        const glm::vec3 dir,
        uint32_t collisionMask) const
    {
        ASSERT_WT();

        if (!isEnabled()) return { false, 0.f };

        const auto& hit = rayCastClosest(
            pos + glm::vec3{ 0.f, 500.f, 0.f },
            dir,
            1000.f,
            collisionMask,
            pool::NodeHandle::NULL_HANDLE);

        if (!hit.isHit) return { false, 0.f };

        return { true, hit.pos.y };
    }

    std::vector<std::pair<bool, float>> PhysicsSystem::getWorldSurfaceLevels(
        std::span<glm::vec3> positions,
        const glm::vec3 dir,
        uint32_t collisionMask) const
    {
        ASSERT_WT();

        if (!isEnabled()) return {};

        std::vector<glm::vec3> origins;
        for (const auto& pos : positions) {
            origins.push_back(pos - dir * glm::vec3{ 0.f, 200.f, 0.f });
        }

        const auto& castResult = rayCastClosestFromMultiple(
            origins,
            dir,
            500.f,
            collisionMask,
            pool::NodeHandle::NULL_HANDLE);

        std::vector<std::pair<bool, float>> result;

        for (const auto& hit : castResult) {
            if (hit.isHit) {
                result.push_back({ true, hit.pos.y });
            }
            else {
                result.emplace_back(false, 0.f);
            }
        }

        return result;
    }

    void PhysicsSystem::generateObjectMeshes()
    {
        debugCounter++;
        debugCounter = 0;

        auto& dbg = debug::DebugContext::modify();
        auto& physicsDbg = dbg.m_physics;

        if (physicsDbg.m_showObjects) {
            auto meshes = m_meshGenerator->generateMeshes(false);
            physicsDbg.m_meshesWT.store(meshes);
        }
        else {
            std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
            physicsDbg.m_meshesWT.store(tmp);
        }
    }

    physics::RayHit PhysicsSystem::rayCastClosest(
        const glm::vec3& origin,
        const glm::vec3& dir,
        float distance,
        uint32_t collisionMask,
        pool::NodeHandle fromNode) const
    {
        ASSERT_WT();

        if (!m_enabled) return {};

        auto& joltPhysicsSystem = const_cast<JPH::PhysicsSystem&>(getJoltPhysicsSystem());

        // Find the body to exclude
        JPH::BodyID excludeBodyId;
        {
            const auto& it = m_handleToId.find(fromNode);
            if (it != m_handleToId.end()) {
                const auto& obj = m_objects[it->second];
                if (obj.m_body.hasPhysicsBody()) {
                    excludeBodyId = obj.m_body.m_bodyId;
                }
            }
        }

        // Create ray
        JPH::RRayCast ray;
        ray.mOrigin = toJoltR(origin);
        ray.mDirection = toJolt(glm::normalize(dir) * distance);

        // Create filter
        CollisionMaskBodyFilter bodyFilter(collisionMask, excludeBodyId);

        // Cast ray
        JPH::RayCastResult result;
        bool hit = joltPhysicsSystem.GetNarrowPhaseQuery().CastRay(
            ray,
            result,
            JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::NON_MOVING),
            JPH::SpecifiedObjectLayerFilter(ObjectLayers::NON_MOVING),
            bodyFilter);

        if (!hit) {
            // Try moving layer
            hit = joltPhysicsSystem.GetNarrowPhaseQuery().CastRay(
                ray,
                result,
                JPH::SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING),
                JPH::SpecifiedObjectLayerFilter(ObjectLayers::MOVING),
                bodyFilter);
        }

        if (hit) {
            // Get hit position
            JPH::RVec3 hitPos = ray.GetPointOnRay(result.mFraction);

            // Get body and lookup node handle
            const auto& bodyLockInterface = joltPhysicsSystem.GetBodyLockInterface();
            JPH::BodyLockRead lock(bodyLockInterface, result.mBodyID);

            if (lock.Succeeded()) {
                const JPH::Body& body = lock.GetBody();

                // Get normal at hit point
                JPH::Vec3 normal = body.GetWorldSpaceSurfaceNormal(result.mSubShapeID2, hitPos);

                // Lookup object ID from body's user data
                physics::object_id objectId = unpackObjectId(body.GetUserData());

                pool::NodeHandle nodeHandle = pool::NodeHandle::NULL_HANDLE;
                if (objectId > 0 && objectId < m_nodeHandles.size()) {
                    nodeHandle = m_nodeHandles[objectId];
                }

                return {
                    fromJolt(hitPos),
                    fromJolt(normal),
                    nodeHandle,
                    result.mFraction * distance,
                    true
                };
            }
        }

        return {};
    }

    std::vector<physics::RayHit> PhysicsSystem::rayCastClosestToMultiple(
        const glm::vec3& origin,
        const std::vector<glm::vec3>& dirs,
        float distance,
        uint32_t collisionMask,
        pool::NodeHandle fromNode) const
    {
        ASSERT_WT();

        if (!m_enabled) return {};

        std::vector<physics::RayHit> results;
        results.reserve(dirs.size());

        for (const auto& dir : dirs) {
            results.push_back(rayCastClosest(origin, dir, distance, collisionMask, fromNode));
        }

        return results;
    }

    std::vector<physics::RayHit> PhysicsSystem::rayCastClosestFromMultiple(
        std::span<glm::vec3> origins,
        const glm::vec3& dir,
        float distance,
        uint32_t collisionMask,
        pool::NodeHandle fromNode) const
    {
        ASSERT_WT();

        if (!m_enabled) return {};

        std::vector<physics::RayHit> results;
        results.reserve(origins.size());

        for (const auto& origin : origins) {
            results.push_back(rayCastClosest(origin, dir, distance, collisionMask, fromNode));
        }

        return results;
    }

    JPH::PhysicsSystem& PhysicsSystem::getJoltPhysicsSystem()
    {
        return JoltFoundation::get().getPhysicsSystem();
    }

    const JPH::PhysicsSystem& PhysicsSystem::getJoltPhysicsSystem() const
    {
        return JoltFoundation::get().getPhysicsSystem();
    }

    JPH::BodyInterface& PhysicsSystem::getBodyInterface()
    {
        return JoltFoundation::get().getBodyInterface();
    }

    const JPH::BodyInterface& PhysicsSystem::getBodyInterface() const
    {
        return JoltFoundation::get().getBodyInterface();
    }
}
