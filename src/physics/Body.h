#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Axis.h"

#include "Category.h"
#include "size.h"

struct BodyDefinition;

namespace JPH {
    class BodyInterface;
    class PhysicsSystem;
}

namespace physics {
    struct Body {
        Body();

        Body(Body& o) = delete;
        Body(const Body& o) = delete;
        Body(Body&& o) noexcept
        {
            swap(o);
        }
        Body(const BodyDefinition& o) noexcept;

        ~Body();

        Body& operator=(Body& o) = delete;
        Body& operator=(Body&& o) noexcept
        {
            Body tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        Body& operator=(const BodyDefinition& o) noexcept;

        void swap(Body& o) noexcept;

        bool isValid() const noexcept {
            return type != BodyType::none;
        }

        bool hasPhysicsBody() const noexcept {
            return !m_bodyId.IsInvalid();
        }

        void release(JPH::BodyInterface& bodyInterface);

        void create(
            physics::object_id objectId,
            physics::Category category,
            uint32_t collisionMask,
            JPH::PhysicsSystem& physicsSystem,
            const glm::vec3& scale);

        void updatePhysic(
            JPH::BodyInterface& bodyInterface,
            const glm::vec3& nodePivot,
            const glm::vec3& nodePos,
            const glm::quat& nodeRot) const;

        void setPhysicPosition(JPH::BodyInterface& bodyInterface, const glm::vec3& pos) const;
        glm::vec3 getPhysicPosition(const JPH::BodyInterface& bodyInterface) const;

        void setPhysicRotation(JPH::BodyInterface& bodyInterface, const glm::quat& rot) const;
        glm::quat getPhysicRotation(const JPH::BodyInterface& bodyInterface) const;

        void setLinearVelocity(JPH::BodyInterface& bodyInterface, const glm::vec3& vel) const;
        void setAngularVelocity(JPH::BodyInterface& bodyInterface, const glm::vec3& vel) const;

    public:
        // NOTE KI *SCALED* using scale of node
        // size{0] == radius
        // box = x, y, z
        // capsule/cylinder = radiux, half-length
        glm::vec3 size{ 1.f };

        // Local space offset from node position to physics body center
        // e.g., for capsule at feet: offset = (0, halfHeight + radius, 0)
        glm::vec3 offset{ 0.f };

        // Computed base rotation (from axis/front/adjust)
        glm::quat baseRotation{ 1.f, 0.f, 0.f, 0.f };
        glm::quat invBaseRotation{ 1.f, 0.f, 0.f, 0.f };

        // initial values for physics
        glm::vec3 linearVelocity{ 0.f };
        glm::vec3 angularVelocity{ 0.f };

        glm::vec3 angularAxis{ 0.f, 1.f, 0.f };

        // default = dInfinity
        // 0 = don't allow rotate
        float maxAngulerVelocity{ 100.f };

        float density{ 1.f };

        JPH::BodyID m_bodyId;

        physics::BodyType type{ physics::BodyType::none };

        bool forceAxis { false };
        bool kinematic { false };
        bool useAnimatedCenter { false };  // Use animated physics_center socket offset

        // Base rotation components
        glm::vec3 baseAdjust{ 0.f };
        util::Axis baseAxis{ util::Axis::y };
        util::Front baseFront{ util::Front::z };
    };
}
