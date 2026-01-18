#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Category.h"
#include "size.h"

struct ShapeDefinition;

namespace JPH {
    class BodyInterface;
    class PhysicsSystem;
}

namespace physics {
    struct Body;

    // Base rotation to align shapes from Z-axis convention to Jolt's Y-axis convention
    // Jolt capsules/cylinders are aligned along Y-axis
    // Our convention (matching mesh generator): Z-axis
    inline glm::quat getAxisAlignmentRotation(ShapeType type) {
        switch (type) {
        case ShapeType::capsule:
        case ShapeType::cylinder:
            // Rotate -90 degrees around X to transform Z-up to Y-up
            return glm::angleAxis(glm::radians(-90.f), glm::vec3(1.f, 0.f, 0.f));
        default:
            return glm::quat{1.f, 0.f, 0.f, 0.f}; // identity
        }
    }

    struct Shape {
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

        bool isValid() const noexcept {
            return type != ShapeType::none;
        }

        bool hasPhysicsBody() const noexcept {
            return !m_staticBodyId.IsInvalid();
        }

        bool isRay() const noexcept {
            return type == ShapeType::ray;
        }

        // Get the base rotation for axis alignment (Z-axis -> Jolt Y-axis)
        glm::quat getBaseRotation() const noexcept {
            return getAxisAlignmentRotation(type);
        }

        // Get full physics rotation: nodeRot * userRotation * baseRotation
        glm::quat getPhysicsRotation(const glm::quat& nodeRot) const noexcept {
            return nodeRot * rotation * getBaseRotation();
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

        // User-specified rotation offset (applied on top of base rotation)
        glm::quat rotation{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 offset{ 0.f };

        // For standalone shapes (without Body), we create a static body
        // For attached shapes (with Body), this remains invalid and shape is part of body
        mutable JPH::BodyID m_staticBodyId;

        // Shape reference for heightfield (kept alive)
        JPH::RefConst<JPH::Shape> m_heightFieldShape;

        physics::Category category{ physics::Category::none };
        uint32_t collisionMask{ UINT_MAX };

        // Material ID for physics material lookup
        physics::material_id materialId{ 0 };

        physics::ShapeType type{ physics::ShapeType::none };

        bool placeable{ true };
    };
}
