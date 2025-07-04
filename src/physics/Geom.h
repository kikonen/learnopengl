#pragma once

#include <ode/ode.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Category.h"
#include "size.h"

struct GeomDefinition;

namespace physics {
    struct Body;

    struct Geom {
        Geom();
        Geom(Geom&& o) noexcept;
        ~Geom();

        Geom& operator=(Geom&& o) noexcept;
        Geom& operator=(const GeomDefinition& o);

        bool isValid() const noexcept {
            return type != GeomType::none;
        }

        void release();

        void create(
            physics::object_id objectId,
            dWorldID worldId,
            dSpaceID spaceId,
            const glm::vec3& scale,
            dBodyID bodyPhysicId);

        void updatePhysic(
            const glm::vec3& nodePivot,
            const glm::vec3& nodePos,
            const glm::quat& nodeRot) const;

        void setPlane(const glm::vec3& pos, const glm::quat& rot) const;
        void setHeightField(const glm::vec3& pos, const glm::quat& rot) const;

        void setPhysicPosition(const glm::vec3& pos) const
        {
            if (placeable) {
                dGeomSetPosition(physicId, pos.x, pos.y, pos.z);
            }
        }

        // NOTE KI  If the geom is attached to a body,
        // the body's position / rotation pointers will be returned, i.e. the result
        // will be identical to calling dBodyGetPosition or dBodyGetRotation.
        glm::vec3 getPhysicPosition() const
        {
            const dReal* dpos = dGeomGetPosition(physicId);
            return {
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };
        }

        void setPhysicRotation(const glm::quat& nodeRot) const
        {
            if (placeable) {
                const auto& rot = nodeRot * rotation;
                dQuaternion dquat{ rot.w, rot.x, rot.y, rot.z };
                dGeomSetQuaternion(physicId, dquat);
            }
        }

        // NOTE KI  If the geom is attached to a body,
        // the body's position / rotation pointers will be returned, i.e. the result
        // will be identical to calling dBodyGetPosition or dBodyGetRotation.
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

    public:
        // NOTE KI *SCALED* using scale of node
        // box:
        // - size == vec3 (half size of cube)
        // sphere:
        // - size.x == radius
        // capsule/cylinder:
        // - size.x == radius
        // - size.y == length (Half of the length between centers of the caps along the z-axis.)
        glm::vec3 size{ 0.5f };

        glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 offset{ 0.f };

        //glm::vec4 plane{ 0.f, 1.f, 0.f, 0.f };

        mutable dGeomID physicId{ nullptr };
        mutable dHeightfieldDataID heightDataId{ nullptr };

        uint32_t categoryMask{ UINT_MAX };
        uint32_t collisionMask{ UINT_MAX };

        // dContactXX flags for geom
        // TODO KI define "PhysicsMaterial" and refer to it from geom
        physics::material_id materialId{ 0 };

        physics::GeomType type{ physics::GeomType::none };

        bool placeable : 1 { true };
    };
}
