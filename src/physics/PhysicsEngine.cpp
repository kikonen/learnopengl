#include "PhysicsEngine.h"
#include "PhysicsEngine.h"

#include <iostream>

#include <fmt/format.h>

#include "asset/Assets.h"

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
        physics::Object* test{ nullptr };
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
            HitData* hitData = static_cast<HitData*>(data);
            auto& hit = hitData->hits.emplace_back();
            hit.handle = hitData->test->m_nodeHandle;

            hit.pos = {
                static_cast<float>(contact.pos[0]),
                static_cast<float>(contact.pos[1]),
                static_cast<float>(contact.pos[2]) };

            hit.normal = {
                static_cast<float>(contact.pos[0]),
                static_cast<float>(contact.pos[1]),
                static_cast<float>(contact.pos[2]) };

            hit.depth = static_cast<float>(contact.depth);
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
            m_rayId = registerObject();
            auto* obj = getObject(m_rayId);
            obj->m_geom.type = GeomType::ray;
            obj->m_body.kinematic = true;
            obj->m_geom.categoryMask = 0;
            obj->m_geom.collisionMask = 0;
        }

        m_meshGenerator = std::make_unique<physics::MeshGenerator>(*this);
    }

    void PhysicsEngine::update(const UpdateContext& ctx)
    {
        if (!m_enabled) return;

        initTemplates();

        m_elapsedTotal += ctx.m_clock.elapsedSecs;
        if (m_elapsedTotal < m_initialDelay) return;

        std::lock_guard lock{ m_lock };

        preparePending(ctx);

        for (auto* obj : m_updateObjects) {
            obj->updateToPhysics(false);
        }

        const float dtTotal = ctx.m_clock.elapsedSecs + m_remainder;
        const int n = static_cast<int>(dtTotal / STEP_SIZE);
        m_remainder = dtTotal - n * STEP_SIZE;

        if (n > 0) {
            m_invokeCount++;
            m_stepCount += n;

            //std::cout << "n=" << n << '\n';

            //std::cout << ctx.m_clock.elapsedSecs << " - " << n << '\n';

            //std::cout << "[BEFORE]\n";
            //for (const auto& it : m_objects) {
            //    Object* obj = it.second;
            //    if (!obj->m_bodyId) continue;

            //    const dReal* pos = dBodyGetPosition(obj->m_bodyId);
            //    std::cout << pos[0] << ", " << pos[1] << ", " << pos[2] << '\n';
            //}

            for (int i = 0; i < n; i++) {
                if (!*m_alive) return;

                dSpaceCollide(m_spaceId, this, &collisionCallback);
                dWorldQuickStep(m_worldId, STEP_SIZE);
                dJointGroupEmpty(m_contactgroupId);
            }

            //std::cout << "[AFTER]\n";
            //for (const auto& it : m_objects) {
            //    Object* obj = it.second;
            //    if (!obj->m_bodyId) continue;

            //    const dReal* pos = dBodyGetPosition(obj->m_bodyId);
            //    std::cout << pos[0] << ", " << pos[1] << ", " << pos[2] << '\n';
            //}

            for (const auto& obj : m_objects) {
                obj.updateFromPhysics();
            }

            generateObjectMeshes();
        }
    }

    void PhysicsEngine::updateBounds(const UpdateContext& ctx)
    {
        if (!m_enabled) return;
        preparePendingNodes(ctx);

        auto enforce = [&ctx, this](pool::NodeHandle handle) {
            auto* node = handle.toNode();
            if (!node) return;

            const auto* type = node->m_typeHandle.toType();

            if (node->m_instancer) {
                for (auto& state : node->m_instancer->modifyStates()) {
                    enforceBounds(ctx, type, *node, state);
                }
            }
            else {
                enforceBounds(ctx, type, *node, node->modifyState());
            }
        };

        if (!m_enforceBoundsStatic.empty()) {
            std::cout << "static: " << m_enforceBoundsStatic.size() << '\n';
            for (auto& handle : m_enforceBoundsStatic) {
                enforce(handle);
            }
            // NOTE KI static is enforced only once (after initial model setup)
            m_enforceBoundsStatic.clear();
        }
    }

    void PhysicsEngine::preparePending(const UpdateContext& ctx)
    {
        if (m_pending.empty()) return;

        std::unordered_map<physics::physics_id, bool> prepared;

        for (const auto& id : m_pending) {
            auto& obj = m_objects[id - 1];

            obj.create(m_worldId, m_spaceId);

            auto* node = obj.m_nodeHandle.toNode();
            if (node) {
                for (auto& heightMap : m_heightMaps) {
                    if (heightMap.m_origin == node) {
                        obj.m_heightMapId = heightMap.m_id;
                        heightMap.create(m_worldId, m_spaceId, obj);
                    }
                }

                obj.updateToPhysics(false);

                if (obj.m_update) {
                    m_updateObjects.push_back(&obj);
                }

                prepared.insert({ id, true });
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

    void PhysicsEngine::preparePendingNodes(const UpdateContext& ctx)
    {
        if (m_pendingNodes.empty()) return;

        std::vector<pool::NodeHandle> prepared;

        for (auto& handle : m_pendingNodes) {
            auto* node = handle.toNode();
            if (!node) continue;

            if (node->getState().getMatrixLevel() == 0) continue;

            auto* type = node->m_typeHandle.toType();
            if (type->m_flags.staticBounds) {
                m_enforceBoundsStatic.push_back(handle);
            }

            prepared.push_back(node->toHandle());
        }

        if (!prepared.empty()) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_pendingNodes.begin(),
                m_pendingNodes.end(),
                [&prepared](const auto& handle) {
                    const auto& it = std::find_if(
                        prepared.cbegin(),
                        prepared.cend(),
                        [handle](const auto& h) { return h == handle; });
                    return it != prepared.end();
                });
            m_pendingNodes.erase(it, m_pendingNodes.end());
        }
    }

    void PhysicsEngine::enforceBounds(
        const UpdateContext& ctx,
        const mesh::MeshType* type,
        Node& node,
        NodeState& state)
    {
        if (state.m_matrixLevel == state.m_physicsLevel) return;
        state.m_physicsLevel = state.m_matrixLevel;

        const auto& worldPos = state.getWorldPosition();
        glm::vec3 pos = state.getPosition();

        const auto surfaceY = getWorldSurfaceLevel(worldPos);

        auto* parent = node.getParent();

        auto y = surfaceY - parent->getState().getWorldPosition().y;
        //y += state.getScale().y;
        pos.y = y;

        //KI_INFO_OUT(fmt::format(
        //    "({},{}, {}) => {}, {}, {}",
        //    worldPos.x, worldPos.z, worldPos.y, pos.x, pos.z, pos.y));

        state.setPosition(pos);
        //state.setRotation(util::degreesToQuat({ 0.f, 0.f, 0.f }));

        if (state.m_dirty) {
            state.updateModelMatrix(parent->getState());
            auto& nodeState = node.modifyState();
            nodeState.m_dirtySnapshot = true;
        }

        //KI_INFO_OUT(fmt::format("LEVEL: nodeId={}, level={}", node.m_id, level));
    }

    physics::physics_id PhysicsEngine::registerObject()
    {
        auto& obj = m_objects.emplace_back<Object>({});
        obj.m_id = static_cast<physics::physics_id>(m_objects.size());

        m_pending.push_back(obj.m_id);

        return obj.m_id;
    }

    Object* PhysicsEngine::getObject(physics::physics_id id)
    {
        if (id < 1 || id > m_objects.size()) return nullptr;
        return &m_objects[id - 1];
    }

    physics::height_map_id PhysicsEngine::registerHeightMap()
    {
        auto& map = m_heightMaps.emplace_back<HeightMap>({});
        map.m_id = static_cast<physics::height_map_id>(m_heightMaps.size());

        return map.m_id;
    }

    const HeightMap* PhysicsEngine::getHeightMap(physics::height_map_id id) const
    {
        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id - 1];
    }

    HeightMap* PhysicsEngine::modifyHeightMap(physics::height_map_id id)
    {
        if (id < 1 || id > m_heightMaps.size()) return nullptr;
        return &m_heightMaps[id - 1];
    }

    float PhysicsEngine::getWorldSurfaceLevel(const glm::vec3& pos)
    {
        float min = 0.f;
        bool hit = false;

        for (const auto& surface : m_heightMaps) {
            if (!surface.m_id) continue;
            if (!surface.isReady()) continue;

            //if (!surface->withinBounds(pos)) continue;

            const float level = surface.getLevel(pos);
            if (!hit || level > min) min = level;
            hit = true;
        }

        return hit ? min : 0.f;
    }

    void PhysicsEngine::handleNodeAdded(Node* node)
    {
        auto* type = node->m_typeHandle.toType();
        if (!type->m_flags.staticBounds) return;
        m_pendingNodes.push_back(node->toHandle());
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
        glm::vec3 origin,
        glm::vec3 dir,
        float distance,
        uint32_t categoryMask,
        uint32_t collisionMask,
        pool::NodeHandle fromNode)
    {
        if (!m_enabled) return {};

        std::lock_guard lock{ m_lock };

        auto* ray = getObject(m_rayId);
        if (!ray || !ray->m_geom.physicId) return {};

        KI_INFO_OUT(fmt::format(
            "RAY: origin={}, dir={}, dist={}, cat={}, col={}",
            origin, dir, distance, categoryMask, collisionMask));

        const auto rayGeomId = ray->m_geom.physicId;

        HitData hitData;

        dGeomRaySet(rayGeomId, origin.x, origin.y, origin.z, dir.x, dir.y, dir.z);
        dGeomRaySetLength(rayGeomId, distance);
        dGeomSetCategoryBits(rayGeomId, categoryMask);
        dGeomSetCollideBits(rayGeomId, collisionMask);

        for (auto& obj : m_objects) {
            if (rayGeomId == obj.m_geom.physicId) continue;
            if (!obj.m_geom.physicId) continue;
            if (obj.m_nodeHandle == fromNode) continue;

            hitData.test = &obj;

            // NOTE KI dCollide  does not check category/collision bitmask
            dSpaceCollide2(rayGeomId, obj.m_geom.physicId, &hitData, &rayCallback);
        }

        // NOTE KI set mask to "none" to prevent collisions after casting
        dGeomSetCategoryBits(rayGeomId, util::as_integer(physics::Category::none));
        dGeomSetCollideBits(rayGeomId, util::as_integer(physics::Category::none));

        return hitData.hits;
    }
}
