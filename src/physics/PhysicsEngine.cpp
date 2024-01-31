#include "PhysicsEngine.h"

#include <iostream>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/Node.h"
#include "model/NodeTransform.h"

#include "mesh/MeshType.h"

#include "engine/UpdateContext.h"

#include "generator/NodeGenerator.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

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

    void PhysicsEngine::prepare(std::shared_ptr<std::atomic<bool>> alive)
    {
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

    void PhysicsEngine::update(const UpdateContext& ctx)
    {
        if (!m_enabled) return;

        m_initialDelay += ctx.m_clock.elapsedSecs;

        if (m_initialDelay > 10) {
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
                    obj.updateFromPhysics();
                }
            }
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
                for (auto& transform : node->m_instancer->modifyTransforms()) {
                    enforceBounds(ctx, type, *node, transform);
                }
            }
            else {
                enforceBounds(ctx, type, *node, node->modifyTransform());
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

            auto* node = obj.m_nodeHandle.toNode();
            if (!node) continue;

            const auto level = node->getTransform().getMatrixLevel();
            if (obj.m_matrixLevel == level) continue;

            obj.prepare(m_worldId, m_spaceId);
            obj.updateToPhysics(false);

            if (obj.m_update) {
                m_updateObjects.push_back(&obj);
            }

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

    void PhysicsEngine::preparePendingNodes(const UpdateContext& ctx)
    {
        if (m_pendingNodes.empty()) return;

        std::vector<ki::node_id> prepared;

        for (auto& handle : m_pendingNodes) {
            auto* node = handle.toNode();
            if (!node) continue;

            if (node->getTransform().getMatrixLevel() < 0) continue;

            auto* type = node->m_typeHandle.toType();
            if (type->m_flags.staticPhysics) {
                m_enforceBoundsStatic.push_back(handle);
            }
            else {
                m_enforceBoundsDynamic.push_back(handle);
            }

            prepared.push_back(node->getId());
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
                        [nodeId = handle.toId()](const auto& id) { return id == nodeId; });
                    return it != prepared.end();
                });
            m_pendingNodes.erase(it, m_pendingNodes.end());
        }
    }

    void PhysicsEngine::enforceBounds(
        const UpdateContext& ctx,
        const mesh::MeshType* type,
        Node& node,
        NodeTransform& transform)
    {
        if (transform.m_matrixLevel == transform.m_physicsLevel) return;
        transform.m_physicsLevel = transform.m_matrixLevel;

        const auto& worldPos = transform.getWorldPosition();
        glm::vec3 pos = transform.getPosition();

        const auto surfaceY = getWorldSurfaceLevel(worldPos);

        auto* parent = node.getParent();

        auto y = surfaceY - parent->getTransform().getWorldPosition().y;
        y += transform.getScale().y;
        pos.y = y;

        //KI_INFO_OUT(fmt::format(
        //    "({},{}, {}) => {}, {}, {}",
        //    worldPos.x, worldPos.z, worldPos.y, pos.x, pos.z, pos.y));

        transform.setPosition(pos);

        if (transform.m_dirty) {
            transform.updateModelMatrix(parent->getTransform());
            auto& nodeTransform = node.modifyTransform();
            nodeTransform.m_dirtySnapshot = true;
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

    void PhysicsEngine::handleNodeAdded(Node* node)
    {
        auto* type = node->m_typeHandle.toType();
        if (!type->m_flags.enforceBounds) return;
        m_pendingNodes.push_back(node->toHandle());
    }
}
