#include "Object.h"

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/Log.h"
#include "util/Util.h"
#include "util/glm_util.h"

#include "model/Node.h"

namespace {
    constexpr int DIR_X = 1;
    constexpr int DIR_Y = 2;
    constexpr int DIR_Z = 3;

    const glm::quat IDENTITY_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics
{
    Object::Object()
    {
    }

    Object::Object(Object&& o) noexcept
        : m_id{ o.m_id },
        m_update { o.m_update },
        m_body{ o.m_body },
        m_geom{ o.m_geom },
        m_bodyId{ o.m_bodyId },
        m_geomId{ o.m_geomId },
        m_nodeHandle{ o.m_nodeHandle },
        m_matrixLevel{ o.m_matrixLevel }
    {
        // NOTE KI o is moved now
        o.m_geomId = 0;
        o.m_bodyId = 0;
    }

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
        dWorldID worldId,
        dSpaceID spaceId)
    {
        if (ready()) return;

        m_body.invBaseRotation = glm::conjugate(m_body.baseRotation);

        const auto& scale = m_body.scale;

        auto sz = scale * m_geom.size;
        float radius = sz.x;
        float length = sz.y * 2.f;

        if (m_body.type != BodyType::none) {
            m_bodyId = dBodyCreate(worldId);

            dMass mass;

            switch (m_body.type) {
            case BodyType::box: {
                dMassSetBox(&mass, m_body.density, sz.x, sz.y, sz.z);
                break;
            }
            case BodyType::sphere: {
                dMassSetSphere(&mass, m_body.density, radius);
                break;
            }
            case BodyType::capsule: {
                dMassSetCapsule(&mass, m_body.density, DIR_Z, radius, length);
                break;
            }
            case BodyType::cylinder: {
                dMassSetCylinder(&mass, m_body.density, DIR_Z, radius, length);
                break;
            }
            }

            dBodySetMass(m_bodyId, &mass);
        }

        switch (m_geom.type) {
        case GeomType::ray: {
            m_geomId = dCreateRay(spaceId, 0);
            dGeomRaySet(m_geomId, 0, 0, 0, 0, 0, 1);
            break;
        }
        case GeomType::plane: {
            glm::vec3 normal{ 0, 1.f, 0 };
            float dist = 0.f;
            auto plane = m_geom.rotation * glm::vec4(normal, 1.f);

            m_geomId = dCreatePlane(spaceId, plane.x, plane.y, plane.z, dist);
            break;
        }
        case GeomType::box: {
            m_geomId = dCreateBox(spaceId, sz.x, sz.y, sz.z);
            break;
        }
        case GeomType::sphere: {
            m_geomId = dCreateSphere(spaceId, radius);
            break;
        }
        case GeomType::capsule: {
            m_geomId = dCreateCapsule(spaceId, radius, length);
            break;
        }
        case GeomType::cylinder: {
            m_geomId = dCreateCylinder(spaceId, radius, length);
            break;
        }
        }

        if (m_geomId) {
            KI_INFO_OUT(fmt::format("id={}, cat={}, col={}", m_id, m_geom.categoryMask, m_geom.collisionMask));
            dGeomSetCategoryBits(m_geomId, m_geom.categoryMask);
            dGeomSetCollideBits(m_geomId, m_geom.collisionMask);

            // NOTE KI node updates only if body
            if (m_bodyId) {
                dGeomSetBody(m_geomId, m_bodyId);

                const auto& offset = m_geom.offset;
                dGeomSetOffsetPosition(m_geomId, offset.x, offset.y, offset.z);
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
            dBodySetMaxAngularSpeed(m_bodyId, m_body.maxAngulerVelocity);

            if (const auto& q = m_body.baseRotation;
                q != IDENTITY_QUAT)
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
    }

    void Object::updateToPhysics(bool force)
    {
        const auto* node = m_nodeHandle.toNode();
        if (!node) return;

        const auto& state = node->getState();
        const auto level = state.getMatrixLevel();
        if (!force && m_matrixLevel == level) return;
        m_matrixLevel = level;

        const glm::vec3& pos = state.getWorldPosition();
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
            const float length = sz.y * 2.f;

            switch (m_geom.type) {
            case GeomType::plane: {
                dVector4 result;
                dGeomPlaneGetParams(m_geomId, result);

                const glm::vec4 plane{
                    static_cast<float>(result[0]),
                    static_cast<float>(result[1]),
                    static_cast<float>(result[2]),
                    static_cast<float>(result[3]) };
                glm::vec3 normal{ plane };

                // NOTE KI distance into direction of plane normal
                auto dist = glm::dot(normal, pos);

                //KI_INFO_OUT(fmt::format("UPDATED_PLANE: n={}, d={} old_d={}",
                //    normal, dist, plane.w));

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

    void Object::updateFromPhysics() const
    {
        if (!m_bodyId) return;
        if (m_body.kinematic) return;

        auto* node = m_nodeHandle.toNode();
        if (!node) return;

        auto* parent = node->getParent();
        if (!parent) return;

        auto& state = node->modifyState();

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        {
            const dReal* dpos = dBodyGetPosition(m_bodyId);
            const dReal* dquat = dBodyGetQuaternion(m_bodyId);

            pos = glm::vec3{
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };

            rot = glm::quat{
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
            pos -= parent->getState().getWorldPosition();

            // https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
            rot = glm::normalize(rot * m_body.invBaseRotation);

            // NOTE KI project rotation to XZ plane to keep nodes UP
            // => nodes still travel backwards, but not rotating grazily
            if (false) {
                // https://discourse.nphysics.org/t/projecting-a-unitquaternion-on-a-2d-plane/70/4
                const auto rotated = glm::mat3(rot) * state.m_front;
                //const auto front = glm::normalize(glm::vec3(rotated.x, 0, rotated.z));
                const auto rads = glm::atan(rotated.x, rotated.z);
                const auto degrees = glm::degrees(rads);
                rot = util::radiansToQuat(glm::vec3(0, rads, 0));

                dQuaternion quat{ rot.w, rot.x, rot.y, rot.z };
                dBodySetQuaternion(m_bodyId, quat);
            }
            //const auto rotatedFront = rotBase * state.m_front;

            //if (m_geom.type == GeomType::box) {
            //    auto degrees = util::quatToDegrees(rq);
            //    KI_INFO_OUT(fmt::format(
            //        "OBJ_BODY_2: id={}, type={}, pos={}, rot={}, degrees={}",
            //        m_id, util::as_integer(m_geom.type), pos, rot, degrees));
            //}
        }

        state.setPosition(pos);
        state.setRotation(rot);
        //m_node->updateModelMatrix();
    }
}
