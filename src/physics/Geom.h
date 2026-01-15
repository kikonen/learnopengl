#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "Category.h"
#include "size.h"

struct GeomDefinition;

namespace JPH {
    class BodyInterface;
    class PhysicsSystem;
}

namespace physics {
    struct Body;

    struct Geom {
        Geom();
        Geom(Geom& o) = delete;
        Geom(const Geom& o) = delete;
        Geom(Geom&& o) noexcept
        {
            swap(o);
        }
        Geom(const GeomDefinition& o) noexcept;

        ~Geom();

        Geom& operator=(Geom& o) = delete;
        Geom& operator=(Geom&& o) noexcept
        {
            Geom tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        Geom& operator=(const GeomDefinition& o) noexcept;

        void swap(Geom& o) noexcept;

        bool isValid() const noexcept {
            return type != GeomType::none;
        }

        bool hasPhysicsBody() const noexcept {
            return !m_staticBodyId.IsInvalid();
        }

        bool isRay() const noexcept {
            return type == GeomType::ray;
        }

        void release(JPH::BodyInterface& bodyInterface);

        // Create standalone geom (no body attached)
        // For geom-only objects, creates a static body
        void create(
            physics::object_id objectId,
            JPH::PhysicsSystem& physicsSystem,
            const glm::vec3& scale,
            JPH::BodyID attachedBodyId);

        void updatePhysic(
            JPH::BodyInterface& bodyInterface,
            const glm::vec3& nodePivot,
            const glm::vec3& nodePos,
            const glm::quat& nodeRot) const;

        void setPlane(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const;
        void setHeightField(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const;

        void setPhysicPosition(JPH::BodyInterface& bodyInterface, const glm::vec3& pos) const;
        glm::vec3 getPhysicPosition(const JPH::BodyInterface& bodyInterface) const;

        void setPhysicRotation(JPH::BodyInterface& bodyInterface, const glm::quat& nodeRot) const;
        glm::quat getPhysicRotation(const JPH::BodyInterface& bodyInterface) const;

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

        // For standalone geoms (without Body), we create a static body
        // For attached geoms (with Body), this remains invalid and shape is part of body
        mutable JPH::BodyID m_staticBodyId;

        // Shape reference for heightfield (kept alive)
        JPH::RefConst<JPH::Shape> m_heightFieldShape;

        uint32_t categoryMask{ UINT_MAX };
        uint32_t collisionMask{ UINT_MAX };

        // Material ID for physics material lookup
        physics::material_id materialId{ 0 };

        physics::GeomType type{ physics::GeomType::none };

        bool placeable{ true };
    };
}
