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
    //constexpr int DIR_X = 1;
    //constexpr int DIR_Y = 2;
    //constexpr int DIR_Z = 3;

    inline glm::vec3 UP{ 0.f, 1.f, 0.f };

    const glm::quat IDENTITY_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics
{
    Object::Object() = default;

    Object::Object(Object&& o) noexcept
        : m_id{ o.m_id },
        m_update { o.m_update },
        m_body{ o.m_body },
        m_geom{ o.m_geom },
        m_nodeHandle{ o.m_nodeHandle },
        m_matrixLevel{ o.m_matrixLevel }
    {
        // NOTE KI o is moved now
        o.m_geom.physicId = nullptr;
        o.m_body.physicId = nullptr;
    }

    Object::~Object() = default;

    void Object::prepare(
        dWorldID worldId,
        dSpaceID spaceId)
    {
        if (ready()) return;

        glm::vec3 scale{ 1.f };

        // NOtE KI assumping that parent scale does not affect node
        // => 99,9% true for physics nodes, i.e. does not make sense
        // for them to have parent node affecting scale
        const auto* node = m_nodeHandle.toNode();
        if (node) {
            const auto& state = node->getState();
            scale = state.getScale();
        }

        m_body.create(worldId, spaceId, scale);
        m_geom.create(worldId, spaceId, scale, m_body);
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
        const auto& scale = state.getScale();
        const auto& rot = state.getModelRotation() * m_body.baseRotation;

        if (m_body.physicId) {
            dBodySetPosition(m_body.physicId, pos[0], pos[1], pos[2]);

            dQuaternion dquat{ rot.w, rot.x, rot.y, rot.z };
            dBodySetQuaternion(m_body.physicId, dquat);
        }
        else if (m_geom.physicId)
        {
            // NOTE KI handle bodiless geoms separately
            const auto& sz = m_geom.size;
            const float radius = sz.x;
            const float length = sz.y * 2.f;

            switch (m_geom.type) {
            case GeomType::plane: {
                //dVector4 result;
                //dGeomPlaneGetParams(m_geom.physicId, result);

                //const glm::vec4 plane{
                //    static_cast<float>(result[0]),
                //    static_cast<float>(result[1]),
                //    static_cast<float>(result[2]),
                //    static_cast<float>(result[3]) };
                //glm::vec3 normal{ plane };

                //KI_INFO_OUT(fmt::format("UPDATED_PLANE: n={}, d={} old_d={}",
                //    normal, dist, plane.w));

                glm::vec3 normal{ UP };
                auto plane = rot * glm::vec4(normal, 1.f);

                // NOTE KI distance into direction of plane normal
                auto dist = glm::dot(normal, pos);

                dGeomPlaneSetParams(m_geom.physicId, plane.x, plane.y, plane.z, dist);
                break;
            }
            //case GeomType::box: {
            //    dGeomBoxSetLengths(m_geom.physicId, sz.x, sz.y, sz.z);
            //    break;
            //}
            //case GeomType::sphere: {
            //    dGeomSphereSetRadius(m_geom.physicId, radius);
            //    break;
            //}
            //case GeomType::capsule: {
            //    dGeomCapsuleSetParams(m_geom.physicId, radius, length);
            //    break;
            //}
            //case GeomType::cylinder: {
            //    dGeomCylinderSetParams(m_geom.physicId, radius, length);
            //    break;
            //}
            }
        }
    }

    void Object::updateFromPhysics() const
    {
        if (!m_body.physicId) return;
        if (m_body.kinematic) return;

        auto* node = m_nodeHandle.toNode();
        if (!node) return;

        auto* parent = node->getParent();
        if (!parent) return;

        auto& state = node->modifyState();

        glm::vec3 pos{ 0.f };
        glm::quat rot{ 1.f, 0.f, 0.f, 0.f };
        {
            pos = m_body.getPhysicPosition();
            rot = m_body.getPhysicRotation();

            // NOTE KI parent *SHOULD* be root (== null) for all physics nodes
            // => otherwise math does not make sense
            pos -= parent->getState().getWorldPosition();

            // https://danceswithcode.net/engineeringnotes/quaternions/quaternions.html
            rot = glm::normalize(rot * m_body.invBaseRotation);

            //// NOTE KI project rotation to XZ plane to keep nodes UP
            //// => nodes still travel backwards, but not rotating grazily
            //if (false) {
            //    // https://discourse.nphysics.org/t/projecting-a-unitquaternion-on-a-2d-plane/70/4
            //    const auto rotated = glm::mat3(rot) * state.m_front;
            //    //const auto front = glm::normalize(glm::vec3(rotated.x, 0, rotated.z));
            //    const auto rads = glm::atan(rotated.x, rotated.z);
            //    const auto degrees = glm::degrees(rads);
            //    rot = util::radiansToQuat(glm::vec3(0, rads, 0));

            //    dQuaternion quat{ rot.w, rot.x, rot.y, rot.z };
            //    dBodySetQuaternion(m_body.physicId, quat);
            //}
            ////const auto rotatedFront = rotBase * state.m_front;

            //if (m_geom.type == GeomType::box) {
            //    auto degrees = util::quatToDegrees(rq);
            //    KI_INFO_OUT(fmt::format(
            //        "OBJ_BODY_2: id={}, type={}, pos={}, rot={}, degrees={}",
            //        m_id, util::as_integer(m_geom.type), pos, rot, degrees));
            //}
        }

        state.setPosition(pos);
        state.setRotation(rot * glm::conjugate(state.m_baseRotation));
        //m_node->updateModelMatrix();
    }
}
