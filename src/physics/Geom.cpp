#include "Geom.h"

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
    Geom::Geom() {}

    Geom::~Geom()
    {
        // Note: release() must be called explicitly with BodyInterface
    }

    Geom::Geom(const GeomDefinition& o) noexcept
    {
        size = o.m_size;
        rotation = o.m_rotation;
        offset = o.m_offset;
        categoryMask = o.m_categoryMask;
        collisionMask = o.m_collisionMask;
        type = o.m_type;
        placeable = o.m_placeable;
    }

    Geom& Geom::operator=(const GeomDefinition& o) noexcept
    {
        Geom tmp(o);
        swap(tmp);
        return *this;
    }

    void Geom::swap(Geom& o) noexcept
    {
        std::swap(size, o.size);
        std::swap(rotation, o.rotation);
        std::swap(offset, o.offset);
        std::swap(m_staticBodyId, o.m_staticBodyId);
        std::swap(m_heightFieldShape, o.m_heightFieldShape);
        std::swap(categoryMask, o.categoryMask);
        std::swap(collisionMask, o.collisionMask);
        std::swap(materialId, o.materialId);
        std::swap(type, o.type);
        std::swap(placeable, o.placeable);
    }

    void Geom::release(JPH::BodyInterface& bodyInterface)
    {
        if (!m_staticBodyId.IsInvalid()) {
            bodyInterface.RemoveBody(m_staticBodyId);
            bodyInterface.DestroyBody(m_staticBodyId);
            m_staticBodyId = JPH::BodyID();
        }
        m_heightFieldShape = nullptr;
    }

    void Geom::create(
        physics::object_id objectId,
        JPH::PhysicsSystem& physicsSystem,
        const glm::vec3& scale,
        JPH::BodyID attachedBodyId)
    {
        if (type == GeomType::none) return;

        // If attached to a body, the shape is part of that body
        // We only create standalone bodies for geom-only objects
        if (!attachedBodyId.IsInvalid()) {
            // Geom is attached to a body - nothing to create here
            // The body already has the shape
            return;
        }

        auto sz = scale * size;
        float radius = sz.x;
        float halfLength = sz.y;

        JPH::RefConst<JPH::Shape> shape;

        switch (type) {
        case GeomType::ray: {
            // Rays in Jolt are NOT shapes - they're query operations
            // Don't create any body for rays
            return;
        }
        case GeomType::plane: {
            // Create a plane shape
            // Note: Jolt planes are infinite
            glm::vec3 normal = rotation * glm::vec3(0, 1, 0);
            JPH::Plane plane(toJolt(normal), 0.0f);
            shape = new JPH::PlaneShape(plane);
            break;
        }
        case GeomType::height_field: {
            // HeightField is handled separately via HeightMap class
            // The shape will be set later when HeightMap::create is called
            return;
        }
        case GeomType::box: {
            shape = new JPH::BoxShape(JPH::Vec3(sz.x, sz.y, sz.z));
            break;
        }
        case GeomType::sphere: {
            shape = new JPH::SphereShape(radius);
            break;
        }
        case GeomType::capsule: {
            shape = new JPH::CapsuleShape(halfLength, radius);
            break;
        }
        case GeomType::cylinder: {
            shape = new JPH::CylinderShape(halfLength, radius);
            break;
        }
        default:
            return;
        }

        if (!shape) return;

        // Create static body for standalone geom
        JPH::ObjectLayer objectLayer = toObjectLayer(categoryMask, false, false);

        JPH::BodyCreationSettings settings(
            shape,
            JPH::RVec3::sZero(),
            JPH::Quat::sIdentity(),
            JPH::EMotionType::Static,
            objectLayer);

        // Set friction and restitution
        settings.mFriction = 0.5f;
        settings.mRestitution = 0.3f;

        // Pack user data
        settings.mUserData = packUserData(objectId, categoryMask, collisionMask);

        JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        m_staticBodyId = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::DontActivate);
    }

    void Geom::updatePhysic(
        JPH::BodyInterface& bodyInterface,
        const glm::vec3& nodePivot,
        const glm::vec3& nodePos,
        const glm::quat& nodeRot) const
    {
        if (type == GeomType::none) return;
        if (type == GeomType::ray) return;

        bool handled = false;
        switch (type) {
        case GeomType::plane: {
            setPlane(bodyInterface, nodePivot, nodeRot * rotation);
            handled = true;
            break;
        }
        case GeomType::height_field: {
            setHeightField(bodyInterface, nodePivot, nodeRot * rotation);
            handled = true;
            break;
        }
        }

        if (!handled) {
            setPhysicPosition(bodyInterface, nodePos);
            setPhysicRotation(bodyInterface, nodeRot);
        }
    }

    void Geom::setPlane(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const
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

    void Geom::setHeightField(JPH::BodyInterface& bodyInterface, const glm::vec3& pos, const glm::quat& rot) const
    {
        if (m_staticBodyId.IsInvalid()) return;

        if (placeable) {
            // HACK KI match current terrain placement logic
            setPhysicPosition(bodyInterface, pos + size * 0.5f);
            setPhysicRotation(bodyInterface, rot);
        }
    }

    void Geom::setPhysicPosition(JPH::BodyInterface& bodyInterface, const glm::vec3& pos) const
    {
        if (m_staticBodyId.IsInvalid()) return;
        if (!placeable) return;

        bodyInterface.SetPosition(m_staticBodyId, toJoltR(pos), JPH::EActivation::DontActivate);
    }

    glm::vec3 Geom::getPhysicPosition(const JPH::BodyInterface& bodyInterface) const
    {
        if (m_staticBodyId.IsInvalid()) return glm::vec3(0.f);
        return fromJolt(bodyInterface.GetPosition(m_staticBodyId));
    }

    void Geom::setPhysicRotation(JPH::BodyInterface& bodyInterface, const glm::quat& nodeRot) const
    {
        if (m_staticBodyId.IsInvalid()) return;
        if (!placeable) return;

        const auto& rot = nodeRot * rotation;
        bodyInterface.SetRotation(m_staticBodyId, toJolt(rot), JPH::EActivation::DontActivate);
    }

    glm::quat Geom::getPhysicRotation(const JPH::BodyInterface& bodyInterface) const
    {
        if (m_staticBodyId.IsInvalid()) return glm::quat(1.f, 0.f, 0.f, 0.f);
        return fromJolt(bodyInterface.GetRotation(m_staticBodyId));
    }
}
