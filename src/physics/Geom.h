#pragma once

#include <ode/ode.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Category.h"

namespace physics {
    struct Body;

    enum class GeomType : std::underlying_type_t<std::byte> {
        none = 0,
        ray,
        plane,
        box,
        sphere,
        capsule,
        cylinder,
    };

    struct Geom {
        dGeomID physicId{ nullptr };

        // NOTE KI *SCALED* using scale of node
        // box:
        // - size == vec3
        // sphere:
        // - size.x == radius
        // capsule/cylinder:
        // - size.x == radius
        // - size.y == length (Half of the length between centers of the caps along the z-axis.)
        glm::vec3 size{ 1.f };

        glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 offset{ 0.f };

        glm::vec4 plane{ 0.f, 1.f, 0.f, 0.f };

        uint32_t categoryMask{ UINT_MAX };
        uint32_t collisionMask{ UINT_MAX };

        // dContactXX flags for geom
        uint32_t contactFlags{ 0 };

        GeomType type{ GeomType::none };

        ~Geom();

        void create(
            dWorldID worldId,
            dSpaceID spaceId,
            const glm::vec3& scale,
            const Body& body);

        void updatePhysic(const glm::vec3& pos, const glm::quat& rot) const
        {
            // NOTE KI handle bodiless geoms separately
            //const auto& sz = size;
            //const float radius = sz.x;
            //const float length = sz.y * 2.f;

            switch (type) {
            case GeomType::plane: {
                setPlane(pos, rot);
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

        void setPlane(const glm::vec3& pos, const glm::quat& rot) const
        {
            const glm::vec3 UP{ 0.f, 1.f, 0.f };

            auto normal = rot * rotation * UP;

            // NOTE KI distance into direction of plane normal
            auto dist = glm::dot(normal, pos);

            dGeomPlaneSetParams(physicId, normal.x, normal.y, normal.z, dist);
        }

        glm::vec3 getPhysicPosition() const
        {
            const dReal* dpos = dGeomGetPosition(physicId);
            return {
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };
        }

        glm::quat getPhysicRotation() const
        {
            dQuaternion dquat;
            dGeomGetQuaternion(physicId, dquat);

            return {
                static_cast<float>(dquat[0]),
                static_cast<float>(dquat[1]),
                static_cast<float>(dquat[2]),
                static_cast<float>(dquat[3]) };
        }
    };
}
