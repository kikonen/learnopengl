#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "util/Axis.h"
#include "util/glm_util.h"

#include "Category.h"
#include "size.h"

struct ShapeDefinition;

namespace JPH
{
    class BodyInterface;
    class PhysicsSystem;
}

namespace physics
{
    struct Body;

    struct Shape
    {
        Shape();
        Shape(Shape& o) = delete;
        Shape(const Shape& o) = delete;
        Shape(Shape&& o) noexcept
        {
            swap(o);
        }
        Shape(const ShapeDefinition& o) noexcept;

        ~Shape();

        Shape& operator=(Shape& o) = delete;
        Shape& operator=(Shape&& o) noexcept
        {
            Shape tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        Shape& operator=(const ShapeDefinition& o) noexcept;

        void swap(Shape& o) noexcept;

        bool isValid() const noexcept
        {
            return type != ShapeType::none;
        }

        bool hasPhysicsBody() const noexcept
        {
            return !m_staticBodyId.IsInvalid();
        }

        bool isRay() const noexcept
        {
            return type == ShapeType::ray;
        }

        // Get base rotation from axis/front/adjust
        glm::quat getBaseRotation() const noexcept
        {
            return util::degreesToQuat(baseAdjust) *
                util::frontToRotation(baseFront) *
                util::axisToRotation(baseAxis);
        }

        // Get full physics rotation: nodeRot * baseRotation
        glm::quat getPhysicsRotation(const glm::quat& nodeRot) const noexcept
        {
            return nodeRot * getBaseRotation();
        }

        void release(JPH::BodyInterface& bodyInterface);

        // Create standalone shape (no body attached)
        // For shape-only objects, creates a static body
        void create(
            physics::object_id objectId,
            JPH::PhysicsSystem& physicsSystem,
            const glm::vec3& scale,
            JPH::BodyID attachedBodyId);

        void updatePhysics(
            JPH::BodyInterface& bodyInterface,
            const glm::vec3& nodePivot,
            const glm::vec3& nodePos,
            const glm::quat& nodeRot) const;

        void setPlane(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const;
        void setHeightField(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const;

        void setPhysicsPosition(JPH::BodyInterface& bodyInterface, const glm::vec3& pos) const;
        glm::vec3 getPhysicsPosition(const JPH::BodyInterface& bodyInterface) const;

        void setPhysicsRotation(JPH::BodyInterface& bodyInterface, const glm::quat& nodeRot) const;
        glm::quat getPhysicsRotation(const JPH::BodyInterface& bodyInterface) const;

    public:
        // NOTE KI *SCALED* using scale of node
        // box:
        // - size == vec3 (half size of cube)
        // sphere:
        // - size.x == radius
        // capsule/cylinder:
        // - size.x == radius
        // - size.y == half length along primary axis
        glm::vec3 size{ 0.5f };

        // Shape reference for heightfield (kept alive)
        JPH::RefConst<JPH::Shape> m_heightFieldShape;

        // For standalone shapes (without Body), we create a static body
        // For attached shapes (with Body), this remains invalid and shape is part of body
        mutable JPH::BodyID m_staticBodyId;

        physics::Category category{ physics::Category::none };
        uint32_t collisionMask{ UINT_MAX };

        // Material ID for physics material lookup
        physics::material_id materialId{ 0 };

        physics::ShapeType type{ physics::ShapeType::none };

        bool placeable{ true };

        // Base rotation components
        util::Axis baseAxis{ util::Axis::y };
        util::Front baseFront{ util::Front::z };
        glm::vec3 baseAdjust{ 0.f };
    };
}
