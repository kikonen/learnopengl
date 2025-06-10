#pragma once

#include <ode/ode.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "size.h"

struct BodyDefinition;

namespace physics {
    struct Body {
        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        // box = x, y, z
        // capsule/cylinder = radiux, half-length
        glm::vec3 size{ 1.f };

        // NOTE KI base rotation to match base rotation of node
        glm::quat baseRotation{ 1.f, 0.f, 0.f, 0.f };
        // reverse rotation of baseRotation
        glm::quat invBaseRotation{ 1.f, 0.f, 0.f, 0.f };

        // initial values for physics
        glm::vec3 linearVelocity{ 0.f };
        glm::vec3 angularVelocity{ 0.f };

        glm::vec3 axis{ 0.f, 1.f, 0.f };

        // default = dInfinity
        // 0 = don't allow rotate
        float maxAngulerVelocity{ 100.f };

        float density{ 1.f };

        dBodyID physicId{ nullptr };

        physics::BodyType type{ physics::BodyType::none };

        bool forceAxis : 1 { false };
        bool kinematic : 1 { false };

        ~Body();
        Body* operator=(const BodyDefinition& o);

        bool isValid() const noexcept {
            return type != BodyType::none;
        }

        void create(
            physics::object_id objectId,
            dWorldID worldId,
            dSpaceID spaceId,
            const glm::vec3& scale);

        void updatePhysic(
            const glm::vec3& nodePivot,
            const glm::vec3& nodePos,
            const glm::quat& nodeRot) const;

        void setPhysicPosition(const glm::vec3& pos) const
        {
            dBodySetPosition(physicId, pos.x, pos.y, pos.z);
        }

        glm::vec3 getPhysicPosition() const
        {
            const dReal* dpos = dBodyGetPosition(physicId);
            return {
                static_cast<float>(dpos[0]),
                static_cast<float>(dpos[1]),
                static_cast<float>(dpos[2]) };
        }

        void setPhysicRotation(const glm::quat& rot) const
        {
            dQuaternion dquat{ rot.w, rot.x, rot.y, rot.z };
            dBodySetQuaternion(physicId, dquat);
        }

        glm::quat getPhysicRotation() const
        {
            const dReal* dquat = dBodyGetQuaternion(physicId);

            return {
                static_cast<float>(dquat[0]),
                static_cast<float>(dquat[1]),
                static_cast<float>(dquat[2]),
                static_cast<float>(dquat[3]) };
        }
    };
}
