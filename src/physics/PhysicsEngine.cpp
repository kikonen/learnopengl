#include "PhysicsEngine.h"
#include "PhysicsEngine.h"

#include <iostream>

#include <fmt/format.h>

#include "asset/Assets.h"

#include "util/thread.h"
#include "util/glm_format.h"
#include "util/Util.h"

#include "model/Node.h"
#include "model/NodeState.h"

#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "render/DebugContext.h"

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

    static physics::PhysicsEngine g_engine;

    size_t debugCounter{ 0 };

    dSurfaceParameters g_surfaceTemplate;

    // NOTE KI shared, only single thread does checking
    dContact g_contacts[MAX_CONTACTS]{};

    struct HitData {
        bool onlyClosest{ false };
        pool::NodeHandle test{};
        std::vector<physics::RayHit> hits{};
    };

    void initSurface(dSurfaceParameters& surface)
    {
        auto& dbg = render::DebugContext::modify();

        // http://monsterden.net/software/ragdoll-pyode-tutorial
        // c.setMu(500) # 0-5 = very slippery, 50-500 = normal, 5000 = very sticky

        int mode = 0;
        if (dbg.m_physics_dContactMu2) mode |= dContactMu2;
        if (dbg.m_physics_dContactSlip1) mode |= dContactSlip1;
        if (dbg.m_physics_dContactSlip2) mode |= dContactSlip2;
        if (dbg.m_physics_dContactRolling) mode |= dContactRolling;
        if (dbg.m_physics_dContactBounce) mode |= dContactBounce;
        if (dbg.m_physics_dContactMotion1) mode |= dContactMotion1;
        if (dbg.m_physics_dContactMotion2) mode |= dContactMotion2;
        if (dbg.m_physics_dContactMotionN) mode |= dContactMotionN;
        if (dbg.m_physics_dContactSoftCFM) mode |= dContactSoftCFM;
        if (dbg.m_physics_dContactSoftERP) mode |= dContactSoftERP;
        if (dbg.m_physics_dContactApprox1) mode |= dContactApprox1;
        if (dbg.m_physics_dContactFDir1) mode |= dContactFDir1;

        surface.mode = mode;

        surface.mu = dbg.m_physics_mu;
        surface.mu2 = dbg.m_physics_mu2;
        surface.rho = dbg.m_physics_rho;
        surface.rho2 = dbg.m_physics_rho2;
        surface.rhoN = dbg.m_physics_rhoN;
        surface.slip1 = dbg.m_physics_slip1;
        surface.slip2 = dbg.m_physics_slip2;
        surface.bounce = dbg.m_physics_bounce;
        surface.bounce_vel = dbg.m_physics_bounce_vel;
        surface.motion1 = dbg.m_physics_motion1;
        surface.motion2 = dbg.m_physics_motion2;
        surface.motionN = dbg.m_physics_motionN;
        surface.soft_erp = dbg.m_physics_soft_erp;
        surface.soft_cfm = dbg.m_physics_soft_cfm;
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
}

namespace physics
{
    static void rayCallback(void* data, dGeomID o1, dGeomID o2) {
        dContactGeom contact;
        if (dCollide(o1, o2, 1, &contact, sizeof(dContactGeom)) != 0) {
            HitData& hitData = *static_cast<HitData*>(data);

            RayHit* lastHit = hitData.hits.empty() ? nullptr : &hitData.hits[0];
            RayHit* hit{ nullptr };

            if (hitData.onlyClosest && lastHit) {
                if (contact.depth < lastHit->depth) {
                    hit = lastHit;
                }
            }
            if (!hit) {
                hit = &hitData.hits.emplace_back();
            }

            hit->handle = hitData.test;

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

        PhysicsEngine* engine = (PhysicsEngine*)data;

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

    PhysicsEngine& PhysicsEngine::get() noexcept
    {
        return g_engine;
    }

    PhysicsEngine::PhysicsEngine()
    {
        m_heightMaps.emplace_back();
        registerObject({}, 0, false, {});
    }

    PhysicsEngine::~PhysicsEngine()
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

    void PhysicsEngine::prepare(std::shared_ptr<std::atomic<bool>> alive)
    {
        m_prepared = true;
        m_alive = alive;

        const auto& assets = Assets::get();

        m_elapsedTotal = 0.f;
        m_initialDelay = assets.physicsInitialDelay;

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

        {
            physics::Object obj{};
            obj.m_geom.type = GeomType::ray;
            obj.m_body.kinematic = true;
            obj.m_geom.categoryMask = 0;
            obj.m_geom.collisionMask = 0;
            m_rayId = registerObject({}, 0, false, obj);
        }

        m_meshGenerator = std::make_unique<physics::MeshGenerator>(*this);
    }

    void PhysicsEngine::updatePrepare(const UpdateContext& ctx)
    {
        if (!m_enabled) return;

        std::lock_guard lock{ m_lock };

        preparePending(ctx);
    }

    void PhysicsEngine::updateObjects(const UpdateContext& ctx)
    {
        if (!m_enabled) return;

        initTemplates();

        m_elapsedTotal += ctx.m_clock.elapsedSecs;
        if (m_elapsedTotal < m_initialDelay) return;

        std::lock_guard lock{ m_lock };

        auto& nodeRegistry = NodeRegistry::get();

        for (const auto id : m_updateObjects) {
            auto& obj = m_objects[id];
            obj.updateToPhysics(m_entityIndeces[id], m_matrixLevels[id], nodeRegistry);
        }

        const float dtTotal = ctx.m_clock.elapsedSecs + m_remainder;
        const int steps = static_cast<int>(dtTotal / STEP_SIZE);
        m_remainder = dtTotal - steps * STEP_SIZE;

        if (steps > 0)
        {
            m_invokeCount++;
            m_stepCount += steps;

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

            generateObjectMeshes();
        }
    }

    void PhysicsEngine::preparePending(const UpdateContext& ctx)
    {
        if (m_pending.empty()) return;

        auto& nodeRegistry = NodeRegistry::get();
        std::unordered_map<physics::object_id, bool> prepared;

        for (const auto& id : m_pending) {
            auto& obj = m_objects[id];
            auto* node = m_nodeHandles[id].toNode();

            if (node) {
                auto entityIndex = m_entityIndeces[id];
                obj.create(entityIndex, m_worldId, m_spaceId, nodeRegistry);

                for (auto& heightMap : m_heightMaps) {
                    if (heightMap.m_origin == node) {
                        obj.m_geom.heightMapId = heightMap.m_id;
                        heightMap.create(m_worldId, m_spaceId, obj);
                    }
                }

                obj.updateToPhysics(entityIndex, m_matrixLevels[id], nodeRegistry);
                prepared.insert({ id, true });
            }
            else {
                obj.create(0, m_worldId, m_spaceId, nodeRegistry);
            }
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

    physics::object_id PhysicsEngine::registerObject(
        pool::NodeHandle nodeHandle,
        uint32_t entityIndex,
        bool update,
        const physics::Object& src)
    {
        auto id = static_cast<physics::object_id>(m_objects.size());

        m_nodeHandles.push_back(nodeHandle);
        m_entityIndeces.push_back(entityIndex);
        m_objects.push_back(src);
        m_pending.push_back(id);
        m_matrixLevels.push_back(0);
        if (update) {
            m_updateObjects.push_back(id);
        }

        return id;
    }

    Object* PhysicsEngine::getObject(physics::object_id id)
    {
        if (id < 1 || id > m_objects.size()) return nullptr;
        return &m_objects[id];
    }

    physics::height_map_id PhysicsEngine::registerHeightMap()
    {
        auto& map = m_heightMaps.emplace_back<HeightMap>({});
        map.m_id = static_cast<physics::height_map_id>(m_heightMaps.size() - 1);

        return map.m_id;
    }

    const HeightMap* PhysicsEngine::getHeightMap(physics::height_map_id id) const
    {
        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id];
    }

    HeightMap* PhysicsEngine::modifyHeightMap(physics::height_map_id id)
    {
        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id];
    }

    dGeomID PhysicsEngine::addGeom(const physics::Geom& geom)
    {
        ASSERT_WT();

        dGeomID physicId{ nullptr };
        {
            Geom g = geom;
            g.create(
                m_worldId,
                m_spaceId,
                glm::vec3{ 1.f },
                nullptr);
            physicId = g.physicId;
            g.physicId = nullptr;
        }

        return physicId;
    }

    void PhysicsEngine::removeGeom(dGeomID physicId)
    {
        ASSERT_WT();
        if (physicId) {
            dGeomDestroy(physicId);
        }
    }

    std::pair<bool, float> PhysicsEngine::getWorldSurfaceLevel(
        const glm::vec3& pos,
        uint32_t categoryMask,
        uint32_t collisionMask)
    {
        if (!isEnabled()) return { false, 0.f };

        const auto& hits = rayCast(
            pos + glm::vec3{ 0.f, 500.f, 0.f },
            { 0.f, -1.f, 0.f },
            1000.f,
            categoryMask,
            collisionMask,
            pool::NodeHandle::NULL_HANDLE,
            true);

        if (hits.empty()) return { false, 0.f };

        return { true, hits[0].pos.y };
    }

    std::vector<std::pair<bool, float>> PhysicsEngine::getWorldSurfaceLevels(
        std::span<glm::vec3> positions,
        const glm::vec3 dir,
        uint32_t collisionMask)
    {
        if (!isEnabled()) return {};

        std::vector<glm::vec3> origins;
        for (const auto& pos : positions) {
            origins.push_back(pos - dir * glm::vec3{ 0.f, 200.f, 0.f });
        }

        const auto& castResult = rayCast(
            origins,
            dir,
            500.f,
            physics::mask(physics::Category::ray_test),
            collisionMask,
            pool::NodeHandle::NULL_HANDLE);

        std::vector<std::pair<bool, float>> result;

        for (const auto& [success, hit]: castResult) {
            if (success) {
                result.push_back({ true, hit.pos.y });
            }
            else {
                result.emplace_back(false, 0.f);
            }
        }
        return result;
    }

    void PhysicsEngine::generateObjectMeshes()
    {
        debugCounter++;
        //if (debugCounter < 2) return;
        debugCounter = 0;

        auto& dbg = render::DebugContext::modify();

        auto meshes = m_meshGenerator->generateMeshes();
        dbg.m_physicsMeshes.swap(meshes);
    }

    std::vector<physics::RayHit> PhysicsEngine::rayCast(
        const glm::vec3& origin,
        const glm::vec3& dir,
        float distance,
        uint32_t categoryMask,
        uint32_t collisionMask,
        pool::NodeHandle fromNode,
        bool onlyClosest)
    {
        if (!m_enabled) return {};

        std::lock_guard lock{ m_lock };

        auto* ray = getObject(m_rayId);
        if (!ray || !ray->m_geom.physicId) return {};

        //KI_INFO_OUT(fmt::format(
        //    "RAY: origin={}, dir={}, dist={}, cat={}, col={}",
        //    origin, dir, distance, categoryMask, collisionMask));

        const auto rayGeomId = ray->m_geom.physicId;

        HitData hitData;
        hitData.onlyClosest = onlyClosest;

        dGeomRaySet(rayGeomId, origin.x, origin.y, origin.z, dir.x, dir.y, dir.z);
        dGeomRaySetLength(rayGeomId, distance);
        dGeomSetCategoryBits(rayGeomId, categoryMask);
        dGeomSetCollideBits(rayGeomId, collisionMask);

        const auto count = m_objects.size();
        for (int i = 0; i < count; i++) {
            const auto& obj = m_objects[i];
            const auto& nodeHandle = m_nodeHandles[i];

            if (rayGeomId == obj.m_geom.physicId) continue;
            if (!obj.m_geom.physicId) continue;
            if (nodeHandle == fromNode) continue;

            hitData.test = nodeHandle;

            // NOTE KI dCollide  does not check category/collision bitmask
            dSpaceCollide2(rayGeomId, obj.m_geom.physicId, &hitData, &rayCallback);

            // NOTE KI *cannot* do early exit for onlyClosest
            // since closest hit !== first hit
        }

        // NOTE KI set mask to "none" to prevent collisions after casting
        dGeomSetCategoryBits(rayGeomId, util::as_integer(physics::Category::none));
        dGeomSetCollideBits(rayGeomId, util::as_integer(physics::Category::none));

        return hitData.hits;
    }

    std::vector<std::pair<bool, physics::RayHit>> PhysicsEngine::rayCast(
        std::span<glm::vec3> origins,
        const glm::vec3& dir,
        float distance,
        uint32_t categoryMask,
        uint32_t collisionMask,
        pool::NodeHandle fromNode)
    {
        if (!m_enabled) return {};

        std::lock_guard lock{ m_lock };

        auto* ray = getObject(m_rayId);
        if (!ray || !ray->m_geom.physicId) return {};

        const auto rayGeomId = ray->m_geom.physicId;

        dGeomRaySetLength(rayGeomId, distance);
        dGeomSetCategoryBits(rayGeomId, categoryMask);
        dGeomSetCollideBits(rayGeomId, collisionMask);

        HitData hitData;
        hitData.onlyClosest = true;
        std::vector<std::pair<bool, physics::RayHit>> result;

        for (const auto& origin : origins) {
            //KI_INFO_OUT(fmt::format(
            //    "RAY: origin={}, dir={}, dist={}, cat={}, col={}",
            //    origin, dir, distance, categoryMask, collisionMask));

            dGeomRaySet(rayGeomId, origin.x, origin.y, origin.z, dir.x, dir.y, dir.z);

            const auto count = m_objects.size();
            for (int i = 0; i < count; i++) {
                const auto& obj = m_objects[i];
                const auto& nodeHandle = m_nodeHandles[i];

                if (rayGeomId == obj.m_geom.physicId) continue;
                if (!obj.m_geom.physicId) continue;

                hitData.test = nodeHandle;

                // NOTE KI dCollide  does not check category/collision bitmask
                dSpaceCollide2(rayGeomId, obj.m_geom.physicId, &hitData, &rayCallback);

                // NOTE KI *cannot* do early exit for onlyClosest
                // since closest hit !== first hit
            }

            if (hitData.hits.empty()) {
                result.emplace_back();
            }
            else {
                result.push_back({ true, hitData.hits[0] });
            }
            hitData.hits.clear();
        }

        // NOTE KI set mask to "none" to prevent collisions after casting
        dGeomSetCategoryBits(rayGeomId, util::as_integer(physics::Category::none));
        dGeomSetCollideBits(rayGeomId, util::as_integer(physics::Category::none));

        return result;
    }
}
