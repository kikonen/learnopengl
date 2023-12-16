#include "Object.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model/Node.h"
#include "PhysicsEngine.h"

namespace {
    constexpr int DIR_X = 1;
    constexpr int DIR_Y = 2;
    constexpr int DIR_Z = 3;

    const glm::quat NULL_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics
{
    Object::~Object()
    {
        if (m_geomId) {
            dGeomDestroy(m_geomId);
        }
        if (m_bodyId) {
            dBodyDestroy(m_bodyId);
        }
    }

    void Object::prepare(
        PhysicsEngine* engine,
        Node* node)
    {
        if (ready()) return;

        const auto& sz = m_geom.size;
        const float radius = sz.x;
        const float length = sz.y;

        if (m_body.type != BodyType::none) {
            m_bodyId = dBodyCreate(engine->m_worldId);

            switch (m_body.type) {
            case BodyType::box: {
                dMassSetBox(&m_mass, m_body.density, sz.x, sz.y, sz.z);
                break;
            }
            case BodyType::sphere: {
                dMassSetSphere(&m_mass, m_body.density, radius);
                break;
            }
            case BodyType::capsule: {
                dMassSetCapsule(&m_mass, m_body.density, DIR_Z, radius, length);
                break;
            }
            case BodyType::cylinder: {
                dMassSetCylinder(&m_mass, m_body.density, DIR_Z, radius, length);
                break;
            }
            }

            dBodySetMass(m_bodyId, &m_mass);
        }

        switch (m_geom.type) {
        case GeomType::plane: {
            const auto& q = m_geom.quat;
            //const auto& plane = m_geom.plane;
            //auto plane = glm::vec4(0, 0, 1, 0) * q;
            const auto rotM = glm::toMat4(q);
            auto plane = rotM * glm::vec4(0, 1, 0, 1);
            m_geomId = dCreatePlane(engine->m_spaceId, plane.x, plane.y, plane.z, 0);
            break;
        }
        case GeomType::box: {
            m_geomId = dCreateBox(engine->m_spaceId, sz.x, sz.y, sz.z);
            break;
        }
        case GeomType::sphere: {
            m_geomId = dCreateSphere(engine->m_spaceId, radius);
            break;
        }
        case GeomType::capsule: {
            m_geomId = dCreateCapsule(engine->m_spaceId, radius, length);
            break;
        }
        case GeomType::cylinder: {
            m_geomId = dCreateCylinder(engine->m_spaceId, radius, length);
            break;
        }
        }

        if (m_geomId) {
            // NOTE KI node updates only if body
            if (m_bodyId) {
                dGeomSetBody(m_geomId, m_bodyId);
            }

            //if (const auto& q = m_geom.quat;
            //    m_geom.quat != NULL_QUAT)
            //{
            //    dQuaternion quat{ q.w, q.x, q.y, q.z };
            //    dGeomSetQuaternion(m_geomId, quat);

            //    dQuaternion quat2;
            //    dGeomGetQuaternion(m_geomId, quat2);
            //    int x = 0;
            //}
        }

        if (m_bodyId) {
            if (const auto& q = m_body.quat;
                q != NULL_QUAT)
            {
                dQuaternion quat{ q.w, q.x, q.y, q.z };
                dBodySetQuaternion(m_bodyId, quat);

                const dReal* qp = dBodyGetQuaternion(m_bodyId);

                glm::quat quat2{
                    static_cast<float>(qp[0]),
                    static_cast<float>(qp[1]),
                    static_cast<float>(qp[2]),
                    static_cast<float>(qp[3]) };
                auto deg = util::quatToDegrees(quat2);

                int x = 0;
            }
        }

        m_node = node;
        engine->registerObject(this);

        updateToPhysics(true);
    }

    void Object::updateToPhysics(bool force)
    {
        if (!(force || m_update)) return;

        const glm::vec3& pos = m_node->getWorldPosition();
        {

            if (m_bodyId) {
                dBodySetPosition(m_bodyId, pos[0], pos[1], pos[2]);
                if (m_body.kinematic) {
                    dBodySetKinematic(m_bodyId);
                }
                else {
                    dBodySetDynamic(m_bodyId);
                }
            }
        }

        if (m_geomId) {
            const auto& sz = m_geom.size;
            const float radius = sz.x;
            const float length = sz.y;

            switch (m_geom.type) {
            case GeomType::plane: {
                dVector4 plane;
                dGeomPlaneGetParams(m_geomId, plane);
                // NOTE KI distance into direction of plane normal
                const auto dist = glm::dot(pos, glm::vec3{ plane[0], plane[1], plane[2] });
                dGeomPlaneSetParams(m_geomId, plane[0], plane[1], plane[2], dist);
                break;
            }
            //case GeomType::box: {
            //    dGeomBoxSetLengths(m_geomId, sz.x, sz.y, sz.z);
            //    break;
            //}
            //case GeomType::sphere: {
            //    dGeomSphereSetRadius(m_geomId, radius);
            //    break;
            //}
            //case GeomType::capsule: {
            //    dGeomCapsuleSetParams(m_geomId, radius, length);
            //    break;
            //}
            //case GeomType::cylinder: {
            //    dGeomCylinderSetParams(m_geomId, radius, length);
            //    break;
            //}
            }
        }
    }

    void Object::updateFromPhysics()
    {
        if (!m_bodyId) return;
        if (m_body.kinematic) return;

        const dReal* dpos = dBodyGetPosition(m_bodyId);
        const dReal* dquat = dBodyGetQuaternion(m_bodyId);

        glm::vec3 pos = { dpos[0], dpos[1], dpos[2] };
        glm::quat rot{
            static_cast<float>(dquat[0]),
            static_cast<float>(dquat[1]),
            static_cast<float>(dquat[2]),
            static_cast<float>(dquat[3]) };

        if (pos.y < -400) {
            pos.y = -400;
            dBodySetPosition(m_bodyId, pos[0], pos[1], pos[2]);
        }
        if (pos.y > 400) {
            pos.y = 400;
            dBodySetPosition(m_bodyId, pos[0], pos[1], pos[2]);
        }
        pos -= m_node->getParent()->getWorldPosition();

        // https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
        auto rotBase = glm::normalize(glm::conjugate(m_body.quat) * rot);

        m_node->setPosition(pos);
        m_node->setQuatRotation(rotBase);
    }
}