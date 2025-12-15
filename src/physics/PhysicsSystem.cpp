#include "PhysicsSystem.h"
#include "PhysicsSystem.h"

#include <iostream>

#include <fmt/format.h>

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

namespace {
    constexpr float STEP_SIZE = 0.03f;

    constexpr int MAX_CONTACTS = 4;
    constexpr int CONTACT_GROUP_ID = 0;

    size_t debugCounter{ 0 };

    std::shared_ptr<std::vector<mesh::MeshInstance>> NO_MESHES;

    dSurfaceParameters g_surfaceTemplate;

    // NOTE KI shared, only single thread does checking
    dContact g_contacts[MAX_CONTACTS]{};

    struct HitData {
        bool onlyClosest{ false };
        dGeomID sourceGeomId{ nullptr };
        dGeomID rayGeomId{ nullptr };
        std::vector<physics::GeomHit> hits{};
    };

    void initSurface(dSurfaceParameters& surface)
    {
        auto& dbg = debug::DebugContext::modify();
        auto& physicsDbg = dbg.m_physics;

        // http://monsterden.net/software/ragdoll-pyode-tutorial
        // c.setMu(500) # 0-5 = very slippery, 50-500 = normal, 5000 = very sticky

        int mode = 0;
        if (physicsDbg.m_dContactMu2) mode |= dContactMu2;
        if (physicsDbg.m_dContactSlip1) mode |= dContactSlip1;
        if (physicsDbg.m_dContactSlip2) mode |= dContactSlip2;
        if (physicsDbg.m_dContactRolling) mode |= dContactRolling;
        if (physicsDbg.m_dContactBounce) mode |= dContactBounce;
        if (physicsDbg.m_dContactMotion1) mode |= dContactMotion1;
        if (physicsDbg.m_dContactMotion2) mode |= dContactMotion2;
        if (physicsDbg.m_dContactMotionN) mode |= dContactMotionN;
        if (physicsDbg.m_dContactSoftCFM) mode |= dContactSoftCFM;
        if (physicsDbg.m_dContactSoftERP) mode |= dContactSoftERP;
        if (physicsDbg.m_dContactApprox1) mode |= dContactApprox1;
        if (physicsDbg.m_dContactFDir1) mode |= dContactFDir1;

        surface.mode = mode;

        surface.mu = physicsDbg.m_mu;
        surface.mu2 = physicsDbg.m_mu2;
        surface.rho = physicsDbg.m_rho;
        surface.rho2 = physicsDbg.m_rho2;
        surface.rhoN = physicsDbg.m_rhoN;
        surface.slip1 = physicsDbg.m_slip1;
        surface.slip2 = physicsDbg.m_slip2;
        surface.bounce = physicsDbg.m_bounce;
        surface.bounce_vel = physicsDbg.m_bounce_vel;
        surface.motion1 = physicsDbg.m_motion1;
        surface.motion2 = physicsDbg.m_motion2;
        surface.motionN = physicsDbg.m_motionN;
        surface.soft_erp = physicsDbg.m_soft_erp;
        surface.soft_cfm = physicsDbg.m_soft_cfm;
    }

    void initTemplates()
    {
        initSurface(g_surfaceTemplate);
    }

    inline void setContactSurface(dContact* contacts, size_t count)
    {
        for (int i = 0; i < count; i++)
        {
            contacts[i].surface = g_surfaceTemplate;
        }
    }

    void odeDebugMessage(int errnum, const char* msg, va_list ap)
    {
        KI_DEBUG(fmt::format("PHYSICS: ODE_DEBUG={}, msg={}", errnum, msg));
    }

    void odeInfoMessage(int errnum, const char* msg, va_list ap)
    {
        KI_INFO_OUT(fmt::format("PHYSICS: ODE_INFO={}, msg={}", errnum, msg));
    }

    void odeErrorMessage(int errnum, const char* msg, va_list ap)
    {
        KI_CRITICAL(fmt::format("PHYSICS: ODE_ERROR={}, msg={}", errnum, msg));
    }

    static physics::PhysicsSystem* s_system{ nullptr };
}

namespace physics
{
    void PhysicsSystem::init() noexcept
    {
        assert(!s_system);
        s_system = new PhysicsSystem();
    }

    void PhysicsSystem::release() noexcept
    {
        auto* s = s_system;
        s_system = nullptr;
        delete s;
    }

    PhysicsSystem& PhysicsSystem::get() noexcept
    {
        assert(s_system);
        return *s_system;
    }
}

namespace physics
{
    static void rayCallback(void* data, dGeomID o1, dGeomID o2) {
        dContactGeom contact;
        if (dCollide(o1, o2, 1, &contact, sizeof(dContactGeom)) != 0) {
            HitData& hitData = *static_cast<HitData*>(data);

            if (o1 == hitData.sourceGeomId || o2 == hitData.sourceGeomId) {
                return;
            }

            GeomHit* hit{ nullptr };

            if (hitData.onlyClosest && !hitData.hits.empty()) {
                hit = &hitData.hits[0];
                if (contact.depth >= hit->depth) {
                    // NOTE KI skip no need for update
                    return;
                }
            }
            else {
                hit = &hitData.hits.emplace_back();
            }

            hit->geomId = o1 == hitData.rayGeomId ? o2 : o1;

            hit->pos = {
                static_cast<float>(contact.pos[0]),
                static_cast<float>(contact.pos[1]),
                static_cast<float>(contact.pos[2]) };

            hit->normal = {
                static_cast<float>(contact.normal[0]),
                static_cast<float>(contact.normal[1]),
                static_cast<float>(contact.normal[2]) };

            hit->depth = static_cast<float>(contact.depth);
        }
    }

    static void collisionCallback(void* data, dGeomID o1, dGeomID o2)
    {
        // exit without doing anything if the two bodies are connected by a joint
        dBodyID b1 = dGeomGetBody(o1);
        dBodyID b2 = dGeomGetBody(o2);
        if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
            return;

        PhysicsSystem* engine = (PhysicsSystem*)data;

        const auto worldId = engine->m_worldId;
        const auto groupId = engine->m_contactgroupId;

        dContact* contacts = g_contacts;

        if (int count = dCollide(o1, o2, MAX_CONTACTS, &contacts[0].geom, sizeof(dContact)))
        {
            //auto* obj1 = engine->m_geomToObject[o1];
            //auto* obj2 = engine->m_geomToObject[o2];

            //dMatrix3 RI;
            //dRSetIdentity(RI);

            setContactSurface(contacts, count);
            for (int i = 0; i < count; i++) {
                dJointID c = dJointCreateContact(
                    worldId,
                    groupId,
                    contacts + i);

                dJointAttach(c, b1, b2);
            }
        }
    };

    PhysicsSystem::PhysicsSystem()
    {
        m_heightMaps.emplace_back();
        registerObject({}, 0, false, {});
    }

    PhysicsSystem::~PhysicsSystem()
    {
        m_objects.clear();
        m_heightMaps.clear();

        if (m_spaceId) {
            dSpaceDestroy(m_spaceId);
        }
        if (m_worldId) {
            dWorldDestroy(m_worldId);
        }
        if (m_contactgroupId) {
            dJointGroupDestroy(m_contactgroupId);
        }
        if (m_prepared) {
            dCloseODE();
        }
    }

    void PhysicsSystem::clear()
    {
        ASSERT_RT();

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

        m_rayId = 0;

        m_heightMaps.clear();
        m_heightMapIds.clear();
        m_bodyToObject.clear();
        m_handleToId.clear();

        {
            // NOTE KI register NULL object
            registerObject({}, 0, false, {});
            m_heightMaps.emplace_back();

            {
                physics::Object obj{};
                obj.m_geom.type = GeomType::ray;
                obj.m_body.kinematic = true;
                obj.m_geom.categoryMask = physics::mask(physics::Category::ray);
                obj.m_geom.collisionMask = 0;
                m_rayId = registerObject({}, 0, false, std::move(obj));
            }
        }

        {
            auto& dbg = debug::DebugContext::modify();
            auto& physicsDbg = dbg.m_physics;

            m_meshGenerator->clear();

            std::shared_ptr<std::vector<mesh::MeshInstance>> tmp;
            physicsDbg.m_meshesWT.store(tmp);
            physicsDbg.m_meshesPending.store(tmp);
            physicsDbg.m_meshesRT.store(tmp);
        }
    }

    void PhysicsSystem::prepare(
        const std::shared_ptr<std::atomic_bool>& alive)
    {
        //ASSERT_WT();

        m_prepared = true;
        m_alive = alive;

        const auto& assets = Assets::get();

        m_elapsedTotal = 0.f;
        m_initialDelay = assets.physicsInitialDelay;

        dSetDebugHandler(odeDebugMessage);
        dSetMessageHandler(odeInfoMessage);
        dSetErrorHandler(odeErrorMessage);

        dInitODE2(0);
        m_worldId = dWorldCreate();
        m_spaceId = dHashSpaceCreate(0);
        if (false) {
            dVector3 center{ 200, 0, 200 };
            dVector3 extends{ 1024, 100, 1024 };
            m_spaceId = dQuadTreeSpaceCreate(0, center, extends, 8);
        }
        m_contactgroupId = dJointGroupCreate(0);

        m_gravity = { 0, -2.01f, 0 };

        dWorldSetGravity(m_worldId, m_gravity.x, m_gravity.y, m_gravity.z);

        m_meshGenerator = std::make_unique<physics::MeshGenerator>(*this);
    }

    void PhysicsSystem::updatePrepare(const UpdateContext& ctx)
    {
        ASSERT_WT();

        if (!m_enabled) return;

        //std::lock_guard lock{ m_lock };

        preparePending(ctx);
    }

    void PhysicsSystem::updateObjects(const UpdateContext& ctx)
    {
        ASSERT_WT();

        if (!m_enabled) return;

        initTemplates();

        m_elapsedTotal += ctx.getClock().elapsedSecs;
        if (m_elapsedTotal < m_initialDelay) return;

        auto& dbg = debug::DebugContext::modify();
        auto& physicsDbg = dbg.m_physics;

        //std::lock_guard lock{ m_lock };

        auto& nodeRegistry = NodeRegistry::get();

        bool updatedPhysics = false;
        for (const auto id : m_updateObjects) {
            if (!id) continue;
            auto& obj = m_objects[id];
            updatedPhysics |= obj.updateToPhysics(m_entityIndeces[id], m_matrixLevels[id], nodeRegistry);
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
                for (int i = 0; i < steps; i++) {
                    if (!*m_alive) return;

                    dSpaceCollide(m_spaceId, this, &collisionCallback);
                    dWorldQuickStep(m_worldId, STEP_SIZE);
                    dJointGroupEmpty(m_contactgroupId);
                }

                for (int i = 0; i < m_objects.size(); i++) {
                    auto& obj = m_objects[i];
                    obj.updateFromPhysics(m_entityIndeces[i], nodeRegistry);
                }
            }

            generateObjectMeshes();
        }
        else {
            if (updatedPhysics) {
                //std::cout << "PHYSICS_UPDATE" << "\n";
                generateObjectMeshes();
            }
        }
    }

    void PhysicsSystem::preparePending(const UpdateContext& ctx)
    {
        ASSERT_WT();

        if (m_pending.empty()) return;

        auto& nodeRegistry = NodeRegistry::get();
        std::unordered_map<physics::object_id, bool> prepared;

        for (const auto& id : m_pending) {
            auto& obj = m_objects[id];
            auto* node = m_nodeHandles[id].toNode();

            if (node) {
                auto entityIndex = m_entityIndeces[id];
                obj.create(id, entityIndex, m_worldId, m_spaceId, nodeRegistry);

                m_bodyToObject.insert({ obj.m_body.physicId, id});

                for (auto& heightMap : m_heightMaps) {
                    if (heightMap.m_origin == node) {
                        heightMap.create(m_worldId, m_spaceId, obj);
                        m_heightMapIds.insert({ obj.m_geom.heightDataId, heightMap.m_id});
                    }
                }

                obj.updateToPhysics(entityIndex, m_matrixLevels[id], nodeRegistry);

                m_handleToId.insert({ m_nodeHandles[id], id });
            }
            else {
                obj.create(id, 0, m_worldId, m_spaceId, nodeRegistry);
            }

            m_level++;

            prepared.insert({ id, true });
        }

        if (!prepared.empty()) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
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
        m_matrixLevels.push_back(0);
        m_updateObjects.push_back(update ? id : 0);

        m_pending.push_back(id);

        return id;
    }

     void PhysicsSystem::unregisterObject(
         physics::object_id objectId)
    {
         if (objectId < 1 || objectId > m_objects.size()) return;

         const auto objectIndex = objectId;

         // delete physics objects via overriding by empty data
         m_objects[objectIndex] = std::move(physics::Object{});

         m_nodeHandles[objectIndex] = pool::NodeHandle::NULL_HANDLE;
         m_entityIndeces[objectIndex] = 0;
         m_matrixLevels[objectIndex] = 0;
         m_updateObjects[objectIndex] = 0;

         m_freeIndeces.push_back(objectIndex);

         // Discard possible stale references
         {
             // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
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

    const HeightMap* PhysicsSystem::getHeightMap(dHeightfieldDataID heighgtDataId) const
    {
        ASSERT_WT();

        const auto& it = m_heightMapIds.find(heighgtDataId);
        if (it == m_heightMapIds.end()) return nullptr;

        return &m_heightMaps[it->second];
    }

    HeightMap* PhysicsSystem::modifyHeightMap(physics::height_map_id id)
    {
        ASSERT_WT();

        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id];
    }

    void PhysicsSystem::registerGeom(
        physics::Geom& geom,
        const glm::vec3& scale)
    {
        ASSERT_WT();

        geom.create(
            0,
            m_worldId,
            m_spaceId,
            scale,
            nullptr);
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
        //if (debugCounter < 2) return;
        debugCounter = 0;

        auto& dbg = debug::DebugContext::modify();
        auto& physicsDbg = dbg.m_physics;

        // https://stackoverflow.com/questions/29541387/is-shared-ptr-swap-thread-safe
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

        //std::lock_guard lock{ m_lock };

        const auto* ray = getObject(m_rayId);
        if (!ray || !ray->m_geom.physicId) return {};

        //KI_INFO_OUT(fmt::format(
        //    "RAY: origin={}, dir={}, dist={}, cat={}, col={}",
        //    origin, dir, distance, categoryMask, collisionMask));

        const auto rayGeomId = ray->m_geom.physicId;

        const physics::Object* sourceObject{ nullptr };

        {
            const auto& it = m_handleToId.find(fromNode);
            if (it != m_handleToId.end()) {
                sourceObject = &m_objects[it->second];
            }
        }

        HitData hitData;
        if (sourceObject) {
            hitData.sourceGeomId = sourceObject->m_geom.physicId;
        }
        hitData.rayGeomId = rayGeomId;
        hitData.onlyClosest = true;

        dGeomRaySet(rayGeomId, origin.x, origin.y, origin.z, dir.x, dir.y, dir.z);
        dGeomRaySetLength(rayGeomId, distance);
        dGeomSetCollideBits(rayGeomId, collisionMask);

        dSpaceCollide2(rayGeomId, (dGeomID)m_spaceId, &hitData, &rayCallback);

        // NOTE KI set mask to "none" to prevent collisions after casting
        dGeomSetCategoryBits(rayGeomId, util::as_integer(physics::Category::none));
        dGeomSetCollideBits(rayGeomId, util::as_integer(physics::Category::none));

        if (!hitData.hits.empty())
        {
            const auto& geomHit = hitData.hits[0];
            dBodyID bodyId = dGeomGetBody(geomHit.geomId);
            const auto& it = m_bodyToObject.find(bodyId);
            if (it != m_bodyToObject.end()) {
                return {
                    geomHit.pos,
                    geomHit.normal,
                    m_nodeHandles[it->second],
                    geomHit.depth,
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

        std::vector<physics::RayHit> result;
        result.reserve(dirs.size());

        //std::lock_guard lock{ m_lock };

        const auto* ray = getObject(m_rayId);
        if (!ray || !ray->m_geom.physicId) return {};

        const auto rayGeomId = ray->m_geom.physicId;

        const physics::Object* sourceObject{ nullptr };

        {
            const auto& it = m_handleToId.find(fromNode);
            if (it != m_handleToId.end()) {
                sourceObject = &m_objects[it->second];
            }
        }

        dGeomRaySetLength(rayGeomId, distance);
        dGeomSetCollideBits(rayGeomId, collisionMask);

        HitData hitData;
        if (sourceObject) {
            hitData.sourceGeomId = sourceObject->m_geom.physicId;
        }
        hitData.rayGeomId = rayGeomId;
        hitData.onlyClosest = true;

        for (const auto& dir : dirs) {
            //KI_INFO_OUT(fmt::format(
            //    "RAY: origin={}, dir={}, dist={}, cat={}, col={}",
            //    origin, dir, distance, categoryMask, collisionMask));

            dGeomRaySet(rayGeomId, origin.x, origin.y, origin.z, dir.x, dir.y, dir.z);
            dSpaceCollide2(rayGeomId, (dGeomID)m_spaceId, &hitData, &rayCallback);

            if (hitData.hits.empty()) {
                result.emplace_back();
            }
            else {
                const auto& geomHit = hitData.hits[0];
                dBodyID bodyId = dGeomGetBody(geomHit.geomId);
                const auto& it = m_bodyToObject.find(bodyId);
                if (it != m_bodyToObject.end()) {
                    result.emplace_back(
                        geomHit.pos,
                        geomHit.normal,
                        m_nodeHandles[it->second],
                        geomHit.depth,
                        true);

                }
            }
            hitData.hits.clear();
        }

        // NOTE KI set mask to "none" to prevent collisions after casting
        dGeomSetCategoryBits(rayGeomId, util::as_integer(physics::Category::none));
        dGeomSetCollideBits(rayGeomId, util::as_integer(physics::Category::none));

        return result;
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

        std::vector<physics::RayHit> result;
        result.reserve(origins.size());

        //std::lock_guard lock{ m_lock };

        const auto* ray = getObject(m_rayId);
        if (!ray || !ray->m_geom.physicId) return {};

        const auto rayGeomId = ray->m_geom.physicId;

        const physics::Object* sourceObject{ nullptr };

        {
            const auto& it = m_handleToId.find(fromNode);
            if (it != m_handleToId.end()) {
                sourceObject = &m_objects[it->second];
            }
        }

        dGeomRaySetLength(rayGeomId, distance);
        dGeomSetCollideBits(rayGeomId, collisionMask);

        HitData hitData;
        if (sourceObject) {
            hitData.sourceGeomId = sourceObject->m_geom.physicId;
        }
        hitData.rayGeomId = rayGeomId;
        hitData.onlyClosest = true;

        for (const auto& origin : origins) {
            //KI_INFO_OUT(fmt::format(
            //    "RAY: origin={}, dir={}, dist={}, cat={}, col={}",
            //    origin, dir, distance, categoryMask, collisionMask));

            dGeomRaySet(rayGeomId, origin.x, origin.y, origin.z, dir.x, dir.y, dir.z);
            dSpaceCollide2(rayGeomId, (dGeomID)m_spaceId, &hitData, &rayCallback);

            if (hitData.hits.empty()) {
                result.emplace_back();
            }
            else {
                const auto& geomHit = hitData.hits[0];
                dBodyID bodyId = dGeomGetBody(geomHit.geomId);
                const auto& it = m_bodyToObject.find(bodyId);
                if (it != m_bodyToObject.end()) {
                    result.emplace_back(
                        geomHit.pos,
                        geomHit.normal,
                        m_nodeHandles[it->second],
                        geomHit.depth,
                        true);

                }
            }
            hitData.hits.clear();
        }

        // NOTE KI set mask to "none" to prevent collisions after casting
        dGeomSetCategoryBits(rayGeomId, util::as_integer(physics::Category::none));
        dGeomSetCollideBits(rayGeomId, util::as_integer(physics::Category::none));

        return result;
    }
}
