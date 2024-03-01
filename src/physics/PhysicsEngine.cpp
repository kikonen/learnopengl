#include "PhysicsEngine.h"

#include <iostream>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "engine/UpdateContext.h"

#include "generator/NodeGenerator.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/NodeSnapshotRegistry.h"

#include "ObjectSnapshotRegistry.h"

#include "Surface.h"

namespace {
    constexpr float STEP_SIZE = 0.03f;

    constexpr int MAX_CONTACTS = 8;
    constexpr int CONTACT_GROUP_ID = 0;
}

namespace physics
{
    static void nearCallback(void* data, dGeomID o1, dGeomID o2)
    {
        // exit without doing anything if the two bodies are connected by a joint
        dBodyID b1 = dGeomGetBody(o1);
        dBodyID b2 = dGeomGetBody(o2);
        if (b1 && b2 && dAreConnectedExcluding(b1, b2, dJointTypeContact))
            return;

        // up to MAX_CONTACTS contacts per box-box
        dContact contact[MAX_CONTACTS]{};
        for (int i = 0; i < MAX_CONTACTS; i++)
        {
            contact[i].surface.mode = dContactBounce |
                dContactSlip1 |
                dContactSlip2 |
                dContactSoftCFM |
                dContactSoftERP |
                dContactApprox1;
            contact[i].surface.mu = 30;// dInfinity;
            contact[i].surface.mu2 = 0;
            contact[i].surface.slip1 = 0.7;
            contact[i].surface.slip2 = 0.7;
            contact[i].surface.bounce = 0.6;
            contact[i].surface.bounce_vel = 1.1;
            contact[i].surface.soft_erp = 0.9;
            contact[i].surface.soft_cfm = 0.9;
        }

        if (int numc = dCollide(o1, o2, MAX_CONTACTS, &contact[0].geom, sizeof(dContact)))
        {
            PhysicsEngine* engine = (PhysicsEngine*)data;
            auto* obj1 = engine->m_geomToObject[o1];
            auto* obj2 = engine->m_geomToObject[o2];

            dMatrix3 RI;
            dRSetIdentity(RI);

            for (int i = 0; i < numc; i++) {
                dJointID c = dJointCreateContact(
                    engine->m_worldId,
                    engine->m_contactgroupId,
                    contact + i);

                dJointAttach(c, b1, b2);
            }
        }
    };

    PhysicsEngine& PhysicsEngine::get() noexcept
    {
        static PhysicsEngine s_engine;
        return s_engine;
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

    void PhysicsEngine::prepare(
        std::shared_ptr<std::atomic<bool>> alive,
        Registry* registry)
    {
        m_registry = registry;
        m_objectSnapshotRegistry = m_registry->m_objectSnapshotRegistry;
        m_pendingSnapshotRegistry = m_registry->m_pendingSnapshotRegistry;

        m_prepared = true;
        m_alive = alive;

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
    }

    void PhysicsEngine::updateWT(const UpdateContext& ctx)
    {
        if (!m_enabled) return;

        m_initialDelay += ctx.m_clock.elapsedSecs;

        if (m_initialDelay > 10) {
            preparePendingObjects(ctx);

            if (!m_updateObjects.empty()) {
                m_pendingSnapshotRegistry->withLock([&ctx, this]() {
                    for (auto* obj : m_updateObjects) {
                        auto& snapshot = m_pendingSnapshotRegistry->getSnapshot(obj->m_nodeSnapshotIndex);
                        obj->fromSnapshot(snapshot, false);
                    }
                });
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

                    dSpaceCollide(m_spaceId, this, &nearCallback);
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
                    auto& snapshot = m_objectSnapshotRegistry->modifySnapshot(obj.m_objectSnapshotIndex);
                    obj.toSnapshot(snapshot);
                }
            }
        }
    }

    void PhysicsEngine::updateBounds(const UpdateContext& ctx)
    {
        if (!m_enabled) return;

        preparePendingBounds(ctx);

        auto enforce = [&ctx, this](physics::NodeBounds bounds) {
            auto* node = bounds.m_nodeHandle.toNode();
            if (!node) return;

            const auto* type = node->m_typeHandle.toType();

            if (node->m_instancer) {
                const auto& snapshots = node->m_instancer->getSnapshots(*m_pendingSnapshotRegistry);
                for (const auto& snapshot : snapshots) {
                    enforceBounds(ctx, bounds, type, *node, snapshot);
                }
            }
            else {
                auto& snapshot = m_pendingSnapshotRegistry->getSnapshot(bounds.m_nodeSnapshotIndex);
                enforceBounds(ctx, bounds, type, *node, snapshot);
            }
        };

        if (!m_staticBounds.empty()) {
            std::cout << "static: " << m_staticBounds.size() << '\n';
            m_pendingSnapshotRegistry->withLock([&enforce, this]() {
                for (auto& bounds : m_staticBounds) {
                    enforce(bounds);
                }
            });
            // NOTE KI static is enforced only once (after initial model setup)
            m_staticBounds.clear();
        }

        if (false && !m_dynamicBounds.empty()) {
            //std::cout << "dynamic: " << m_dynamicBounds.size() << '\n';
            m_pendingSnapshotRegistry->withLock([&enforce, this]() {
                for (auto& bounds : m_dynamicBounds) {
                    enforce(bounds);
                }
            });
        }
    }

    void PhysicsEngine::preparePendingObjects(const UpdateContext& ctx)
    {
        std::lock_guard lock(m_pendingLock);

        if (m_pendingObjects.empty()) return;

        std::unordered_map<physics::physics_id, bool> prepared;

        m_pendingSnapshotRegistry->withLock([&prepared, this]() {
            for (const auto& id : m_pendingObjects) {
                auto& obj = m_objects[id - 1];

                auto* node = obj.m_nodeHandle.toNode();
                if (!node) continue;

                auto& snapshot = m_pendingSnapshotRegistry->getSnapshot(obj.m_nodeSnapshotIndex);

                const auto level = snapshot.getMatrixLevel();
                if (obj.m_matrixLevel == level) continue;

                obj.prepare(m_worldId, m_spaceId);
                obj.fromSnapshot(snapshot, false);

                if (obj.m_update) {
                    m_updateObjects.push_back(&obj);
                }

                prepared.insert({ id, true });
            }
        });

        if (!prepared.empty()) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_pendingObjects.begin(),
                m_pendingObjects.end(),
                [&prepared](auto& id) {
                    return prepared.find(id) != prepared.end();
                });
            m_pendingObjects.erase(it, m_pendingObjects.end());
        }
    }

    void PhysicsEngine::preparePendingBounds(const UpdateContext& ctx)
    {
        std::lock_guard lock(m_pendingLock);

        if (m_pendingNodes.empty()) return;

        std::vector<ki::node_id> prepared;

        m_pendingSnapshotRegistry->withLock([&prepared, this]() {
            for (auto& bounds : m_pendingNodes) {
                auto* node = bounds.m_nodeHandle.toNode();

                if (!node) {
                    // NOTE KI deleted node
                    prepared.push_back(bounds.m_nodeHandle.toId());
                    continue;
                }

                auto& snapshot = m_pendingSnapshotRegistry->getSnapshot(bounds.m_nodeSnapshotIndex);
                if (snapshot.getMatrixLevel() == 0) continue;

                if (bounds.m_static) {
                    m_staticBounds.push_back(bounds);
                }
                else {
                    m_dynamicBounds.push_back(bounds);
                }

                prepared.push_back(node->getId());
            }
        });

        if (!prepared.empty()) {
            // https://stackoverflow.com/questions/22729906/stdremove-if-not-working-properly
            const auto& it = std::remove_if(
                m_pendingNodes.begin(),
                m_pendingNodes.end(),
                [&prepared](const auto& bounds) {
                    const auto& it = std::find_if(
                        prepared.cbegin(),
                        prepared.cend(),
                        [nodeId = bounds.m_nodeHandle.toId()](const auto& id) { return id == nodeId; });
                    return it != prepared.end();
                });
            m_pendingNodes.erase(it, m_pendingNodes.end());
        }
    }

    void PhysicsEngine::enforceBounds(
        const UpdateContext& ctx,
        NodeBounds& bounds,
        const mesh::MeshType* type,
        Node& node,
        const Snapshot& nodeSnapshot)
    {
        if (nodeSnapshot.m_matrixLevel == bounds.m_matrixLevel) return;
        bounds.m_matrixLevel = nodeSnapshot.m_matrixLevel;

        const auto& worldPos = nodeSnapshot.getWorldPosition();

        const auto surfaceY = getWorldSurfaceLevel(worldPos);

        auto* parent = node.getParent();

        if (surfaceY != worldPos.y) {
            auto& objectSnapshot = m_objectSnapshotRegistry->modifySnapshot(bounds.m_objectSnapshotIndex);
            objectSnapshot.m_worldPos = worldPos;
            objectSnapshot.m_worldPos.y = surfaceY;
            objectSnapshot.m_dirty = true;
        }
    }

    std::tuple<physics::physics_id, uint32_t> PhysicsEngine::registerObject(
        const physics::Object& src)
    {
        std::lock_guard lock(m_pendingLock);

        auto& dst = m_objects.emplace_back();
        {
            dst.m_objectSnapshotIndex = m_objectSnapshotRegistry->registerSnapshot();
            dst.m_update = src.m_update;
            dst.m_body = src.m_body;
            dst.m_geom = src.m_geom;
            dst.m_nodeHandle = src.m_nodeHandle;
            dst.m_id = static_cast<physics::physics_id>(m_objects.size());
            dst.m_nodeSnapshotIndex = src.m_nodeSnapshotIndex;
        }

        m_pendingObjects.push_back(dst.m_id);

        return { dst.m_id, dst.m_objectSnapshotIndex };
    }

    //Object* PhysicsEngine::getObject(physics::physics_id id)
    //{
    //    if (id < 1 || id > m_objects.size()) return nullptr;
    //    return &m_objects[id - 1];
    //}

    physics::height_map_id PhysicsEngine::registerHeightMap()
    {
        std::lock_guard lock(m_lock);

        auto& map = m_heightMaps.emplace_back<HeightMap>({});
        map.m_id = static_cast<physics::height_map_id>(m_heightMaps.size());

        return map.m_id;
    }

    HeightMap* PhysicsEngine::getHeightMap(physics::height_map_id id)
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
            //if (!surface->withinBounds(pos)) continue;

            const float level = surface.getLevel(pos);
            if (!hit || level > min) min = level;
            hit = true;
        }

        return hit ? min : 0.f;
    }

    void PhysicsEngine::registerBoundsNode(Node* node)
    {
        const auto* type = node->m_typeHandle.toType();
        const auto enforce = type->m_flags.staticBounds || type->m_flags.dynamicBounds;
        if (!enforce) return;

        {
            std::lock_guard lock(m_pendingLock);

            auto& bounds = m_pendingNodes.emplace_back();

            bounds.m_static = type->m_flags.staticBounds;
            bounds.m_objectSnapshotIndex = m_objectSnapshotRegistry->registerSnapshot();
            bounds.m_nodeSnapshotIndex = node->m_snapshotIndex;
            bounds.m_nodeHandle = node->toHandle();

            node->m_objectSnapshotIndex = bounds.m_objectSnapshotIndex;
        }
    }
}
