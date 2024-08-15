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

    // NOTE KI shared, only single thread does checking
    dContact g_contacts[MAX_CONTACTS]{};

    struct HitData {
        physics::Object* test;
        std::vector<physics::RayHit> hits;
    };

    inline void resetContactDefaults()
    {
        // http://monsterden.net/software/ragdoll-pyode-tutorial
        // c.setMu(500) # 0-5 = very slippery, 50-500 = normal, 5000 = very sticky

        // up to MAX_CONTACTS contacts per box-box
        for (int i = 0; i < MAX_CONTACTS; i++)
        {
            g_contacts[i].surface.mode = 0 |
                dContactBounce |
                dContactSlip1 |
                dContactSlip2 |
                dContactSoftCFM |
                dContactSoftERP |
                dContactApprox1;
            g_contacts[i].surface.mu = 30;// dInfinity;
            g_contacts[i].surface.mu2 = 0;
            g_contacts[i].surface.slip1 = 0.7;
            g_contacts[i].surface.slip2 = 0.7;
            g_contacts[i].surface.bounce = 0.6;
            g_contacts[i].surface.bounce_vel = 1.1;
            g_contacts[i].surface.soft_erp = 0.9;
            g_contacts[i].surface.soft_cfm = 0.9;
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

        resetContactDefaults();

        if (int count = dCollide(o1, o2, MAX_CONTACTS, &g_contacts[0].geom, sizeof(dContact)))
        {
            auto* obj1 = engine->m_geomToObject[o1];
            auto* obj2 = engine->m_geomToObject[o2];

            //dMatrix3 RI;
            //dRSetIdentity(RI);

            for (int i = 0; i < count; i++) {
                dJointID c = dJointCreateContact(
                    worldId,
                    groupId,
                    g_contacts + i);

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

        if (!m_enforceBoundsDynamic.empty()) {
            //std::cout << "dynamic: " << m_enforceBoundsDynamic.size() << '\n';
            for (auto& handle : m_enforceBoundsDynamic) {
                enforce(handle);
            }
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
            else {
                m_enforceBoundsDynamic.push_back(handle);
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
        if (!(type->m_flags.staticBounds || type->m_flags.dynamicBounds)) return;
        m_pendingNodes.push_back(node->toHandle());
    }

    void PhysicsEngine::generateObjectMeshes()
    {
        debugCounter++;
        //if (debugCounter < 2) return;
        debugCounter = 0;

        auto& debugContext = render::DebugContext::modify();

        auto meshes = m_meshGenerator->generateMeshes();
        debugContext.m_physicsMeshes.swap(meshes);
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
