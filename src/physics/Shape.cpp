#include "Shape.h"

#include <algorithm>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/PlaneShape.h>

#include "util/debug.h"

#include "component/definition/PhysicsDefinition.h"

#include "jolt_util.h"
#include "JoltFoundation.h"
#include "Body.h"

namespace physics {
    Shape::Shape() {}

    Shape::~Shape()
    {
        // Note: release() must be called explicitly with BodyInterface
    }

    Shape::Shape(const ShapeDefinition& o) noexcept
    {
        size = o.m_size;
        rotation = o.m_rotation;
        offset = o.m_offset;
        category = o.m_category;
        collisionMask = o.m_collisionMask;
        type = o.m_type;
        placeable = o.m_placeable;
    }

    Shape& Shape::operator=(const ShapeDefinition& o) noexcept
    {
        Shape tmp(o);
        swap(tmp);
        return *this;
    }

    void Shape::swap(Shape& o) noexcept
    {
        std::swap(size, o.size);
        std::swap(rotation, o.rotation);
        std::swap(offset, o.offset);
        std::swap(m_staticBodyId, o.m_staticBodyId);
        std::swap(m_heightFieldShape, o.m_heightFieldShape);
        std::swap(category, o.category);
        std::swap(collisionMask, o.collisionMask);
        std::swap(materialId, o.materialId);
        std::swap(type, o.type);
        std::swap(placeable, o.placeable);
    }

    void Shape::release(JPH::BodyInterface& bodyInterface)
    {
        if (!m_staticBodyId.IsInvalid()) {
            bodyInterface.RemoveBody(m_staticBodyId);
            bodyInterface.DestroyBody(m_staticBodyId);
            m_staticBodyId = JPH::BodyID();
        }
        m_heightFieldShape = nullptr;
    }

    void Shape::create(
        physics::object_id objectId,
        JPH::PhysicsSystem& physicsSystem,
        const glm::vec3& scale,
        JPH::BodyID attachedBodyId)
    {
        if (type == ShapeType::none) return;

        // If attached to a body, the shape is part of that body
        // We only create standalone bodies for shape-only objects
        if (!attachedBodyId.IsInvalid()) {
            // Shape is attached to a body - nothing to create here
            // The body already has the shape
            return;
        }

        auto sz = scale * size;
        float radius = sz.x;
        float halfLength = sz.y;

        JPH::RefConst<JPH::Shape> shape;

        switch (type) {
        case ShapeType::ray: {
            // Rays in Jolt are NOT shapes - they're query operations
            // Don't create any body for rays
            return;
        }
        case ShapeType::plane: {
            // Create a plane shape
            // Note: Jolt planes are infinite
            glm::vec3 normal = rotation * glm::vec3(0, 1, 0);
            JPH::Plane plane(toJolt(normal), 0.0f);
            shape = new JPH::PlaneShape(plane);
            break;
        }
        case ShapeType::height_field: {
            // HeightField is handled separately via HeightMap class
            // The shape will be set later when HeightMap::create is called
            return;
        }
        case ShapeType::box: {
            shape = new JPH::BoxShape(JPH::Vec3(sz.x, sz.y, sz.z));
            break;
        }
        case ShapeType::sphere: {
            shape = new JPH::SphereShape(radius);
            break;
        }
        case ShapeType::capsule: {
            // Jolt capsule: halfHeight along Y-axis, radius
            shape = new JPH::CapsuleShape(halfLength, radius);
            break;
        }
        case ShapeType::cylinder: {
            // Jolt cylinder: halfHeight along Y-axis, radius
            shape = new JPH::CylinderShape(halfLength, radius);
            break;
        }
        default:
            return;
        }

        if (!shape) return;

        // Create static body for standalone shape
        JPH::ObjectLayer objectLayer = toObjectLayer(category, false, false);

        // Apply base rotation for axis alignment
        glm::quat initialRot = rotation * getBaseRotation();

        JPH::BodyCreationSettings settings(
            shape,
            JPH::RVec3::sZero(),
            toJolt(initialRot),
            JPH::EMotionType::Static,
            objectLayer);

        // Set friction and restitution
        settings.mFriction = 0.5f;
        settings.mRestitution = 0.3f;

        // Pack user data
        settings.mUserData = packUserData(objectId, category, collisionMask);

        JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        m_staticBodyId = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
    }

    void Shape::updatePhysics(
        JPH::BodyInterface& bodyInterface,
        const glm::vec3& nodePivot,
        const glm::vec3& nodePos,
        const glm::quat& nodeRot) const
    {
        if (type == ShapeType::none) return;
        if (type == ShapeType::ray) return;

        bool handled = false;
        switch (type) {
        case ShapeType::plane: {
            setPlane(bodyInterface, nodePivot, getPhysicsRotation(nodeRot));
            handled = true;
            break;
        }
        case ShapeType::height_field: {
            setHeightField(bodyInterface, nodePivot, getPhysicsRotation(nodeRot));
            handled = true;
            break;
        }
        }

        if (!handled) {
            setPhysicsPosition(bodyInterface, nodePos);
            setPhysicsRotation(bodyInterface, nodeRot);
        }
    }

    void Shape::setPlane(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const
    {
        if (m_staticBodyId.IsInvalid()) return;

        // For planes, we need to recreate the shape since Jolt planes are defined at creation
        // For now, just update position/rotation
        bodyInterface.SetPositionAndRotation(
            m_staticBodyId,
            toJoltR(pos),
            toJolt(rot),
            JPH::EActivation::DontActivate);
    }

    void Shape::setHeightField(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const
    {
        if (m_staticBodyId.IsInvalid()) return;

        if (placeable) {
            // HACK KI match current terrain placement logic
            setPhysicsPosition(bodyInterface, pos + size * 0.5f);
            setPhysicsRotation(bodyInterface, glm::quat{1.f, 0.f, 0.f, 0.f}); // HeightField doesn't need base rotation
        }
    }

    void Shape::setPhysicsPosition(JPH::BodyInterface& bodyInterface, const glm::vec3& pos) const
    {
        if (m_staticBodyId.IsInvalid()) return;
        if (!placeable) return;

        bodyInterface.SetPosition(m_staticBodyId, toJoltR(pos), JPH::EActivation::DontActivate);
    }

    glm::vec3 Shape::getPhysicsPosition(const JPH::BodyInterface& bodyInterface) const
    {
        if (m_staticBodyId.IsInvalid()) return glm::vec3(0.f);
        return fromJolt(bodyInterface.GetPosition(m_staticBodyId));
    }

    void Shape::setPhysicsRotation(JPH::BodyInterface& bodyInterface, const glm::quat& nodeRot) const
    {
        if (m_staticBodyId.IsInvalid()) return;
        if (!placeable) return;

        // Apply full rotation: node * user * base
        const auto& rot = getPhysicsRotation(nodeRot);
        bodyInterface.SetRotation(m_staticBodyId, toJolt(rot), JPH::EActivation::DontActivate);
    }

    glm::quat Shape::getPhysicsRotation(const JPH::BodyInterface& bodyInterface) const
    {
        if (m_staticBodyId.IsInvalid()) return glm::quat(1.f, 0.f, 0.f, 0.f);
        // Note: This returns the raw physics rotation including base rotation
        return fromJolt(bodyInterface.GetRotation(m_staticBodyId));
    }
}
