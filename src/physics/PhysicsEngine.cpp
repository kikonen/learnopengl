#include "PhysicsEngine.h"

#include <iostream>

#include <fmt/format.h>

#include "util/glm_format.h"

#include "model/NodeInstance.h"

#include "engine/UpdateContext.h"

#include "generator/NodeGenerator.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "Surface.h"

namespace {
    constexpr float STEP_SIZE = 0.01;

    constexpr int MAX_CONTACTS = 10;
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
            auto* obj1 = engine->m_objects[o1];
            auto* obj2 = engine->m_objects[o2];

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

    PhysicsEngine::PhysicsEngine(const Assets& assets)
        : m_assets(assets)
    {
    }

    PhysicsEngine::~PhysicsEngine()
    {
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

    void PhysicsEngine::prepare()
    {
        m_prepared = true;
        dInitODE2(0);
        m_worldId = dWorldCreate();
        m_spaceId = dHashSpaceCreate(0);
        m_contactgroupId = dJointGroupCreate(0);

        m_gravity = { 0, -2.01f, 0 };

        dWorldSetGravity(m_worldId, m_gravity.x, m_gravity.y, m_gravity.z);
    }

    void PhysicsEngine::update(const UpdateContext& ctx)
    {
        if (!m_enabled)  return;

        m_initialDelay += ctx.m_clock.elapsedSecs;

        if (m_initialDelay > 10) {
            const float dtTotal = ctx.m_clock.elapsedSecs + m_remainder;
            const long n = dtTotal / STEP_SIZE;
            m_remainder = dtTotal - n * STEP_SIZE;

            if (n > 0) {
                //std::cout << ctx.m_clock.elapsedSecs << " - " << n << '\n';

                //std::cout << "[BEFORE]\n";
                //for (const auto& it : m_objects) {
                //    Object* obj = it.second;
                //    if (!obj->m_bodyId) continue;

                //    const dReal* pos = dBodyGetPosition(obj->m_bodyId);
                //    std::cout << pos[0] << ", " << pos[1] << ", " << pos[2] << '\n';
                //}

                for (int i = 0; i < n; i++) {
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

                for (const auto& it : m_objects) {
                    Object* obj = it.second;
                    if (!(obj->m_bodyId || obj->m_node)) continue;

                    obj->updateFromPhysics();
                }
            }
        }

        for (const auto& all : ctx.m_registry->m_nodeRegistry->physicsNodes) {
            for (const auto& it : all.second) {
                const MeshType& type = *it.first.type;

                if (type.m_flags.physics) {
                    for (auto& node : it.second) {
                        if (node->m_instancer) {
                            for (auto& instance : node->m_instancer->getInstances()) {
                                updateNode(ctx, type, *node, instance);
                            }
                        }
                        else {
                            updateNode(ctx, type, *node, node->getInstance());
                        }
                    }
                }

                if (type.m_flags.enforceBounds) {
                    for (auto& node : it.second) {
                        if (node->m_instancer) {
                            for (auto& instance : node->m_instancer->getInstances()) {
                                enforceBounds(ctx, type, *node, instance);
                            }
                        }
                        else {
                            enforceBounds(ctx, type, *node, node->getInstance());
                        }
                    }
                }
            }
        }
    }

    void PhysicsEngine::enforceBounds(
        const UpdateContext& ctx,
        const MeshType& type,
        Node& node,
        NodeInstance& instance)
    {
        int physicsLevel = type.m_flags.staticPhysics ? m_staticPhysicsLevel : m_physicsLevel;
        if (instance.m_physicsLevel == m_staticPhysicsLevel &&
            instance.m_matrixLevel == instance.m_physicsMatrixLevel) return;

        const auto& worldPos = instance.getWorldPosition();
        glm::vec3 pos = instance.getPosition();

        auto surfaceY = getWorldSurfaceLevel(worldPos);

        auto* parent = node.getParent();

        auto y = surfaceY - parent->getWorldPosition().y;
        y += instance.getScale().y;
        pos.y = y;

        //KI_INFO_OUT(fmt::format(
        //    "({},{}, {}) => {}, {}, {}",
        //    worldPos.x, worldPos.z, worldPos.y, pos.x, pos.z, pos.y));

        instance.setPosition(pos);

        if (instance.m_dirty) {
            instance.updateModelMatrix(parent->getInstance());
        }

        instance.m_physicsMatrixLevel = instance.m_matrixLevel;
        instance.m_physicsLevel = physicsLevel;

        //KI_INFO_OUT(fmt::format("LEVEL: nodeId={}, level={}", node.m_objectID, level));
    }

    void PhysicsEngine::updateNode(
        const UpdateContext& ctx,
        const MeshType& type,
        Node& node,
        NodeInstance& instance)
    {
    }

    void PhysicsEngine::registerObject(Object* obj)
    {
        m_objects.insert({ obj->m_geomId, obj });
    }

    Surface* PhysicsEngine::registerSurface(std::unique_ptr<Surface> surface)
    {
        m_surfaces.push_back(std::move(surface));
        m_physicsLevel++;
        m_staticPhysicsLevel++;
        return m_surfaces.back().get();
    }

    float PhysicsEngine::getWorldSurfaceLevel(const glm::vec3& pos)
    {
        float min = 0.f;
        bool hit = false;

        for (auto& surface : m_surfaces) {
            //if (!surface->withinBounds(pos)) continue;

            float level = surface->getLevel(pos);
            if (!hit || level > min) min = level;
            hit = true;
        }

        return hit ? min : 0.f;
    }
}
