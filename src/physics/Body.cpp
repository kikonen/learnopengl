#include "Body.h"

#include <algorithm>

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>

#include "util/glm_util.h"

#include "component/definition/PhysicsDefinition.h"

#include "jolt_util.h"
#include "JoltFoundation.h"

namespace {
    constexpr int DIR_X = 1;
    constexpr int DIR_Y = 2;
    constexpr int DIR_Z = 3;

    const glm::quat IDENTITY_QUAT{ 0.f, 0.f, 0.f, 0.f };
}

namespace physics {
    Body::Body() = default;

    Body::Body(const BodyDefinition& o) noexcept
    {
        size = o.m_size;

        baseRotation = o.m_baseRotation;
        invBaseRotation = glm::conjugate(baseRotation);

        linearVelocity = o.m_linearVelocity;
        angularVelocity = o.m_angularVelocity;

        axis = o.m_axis;
        maxAngulerVelocity = o.m_maxAngulerVelocity;

        density = o.m_density;

        type = o.m_type;

        forceAxis = o.m_forceAxis;
        kinematic = o.m_kinematic;
    }

    Body::~Body()
    {
        // Note: release() must be called explicitly with BodyInterface
        // We can't call it here because we don't have access to PhysicsSystem
    }

    Body& Body::operator=(const BodyDefinition& o) noexcept
    {
        Body tmp(o);
        swap(tmp);
        return *this;
    }

    void Body::swap(Body& o) noexcept
    {
        std::swap(size, o.size);
        std::swap(baseRotation, o.baseRotation);
        std::swap(invBaseRotation, o.invBaseRotation);
        std::swap(linearVelocity, o.linearVelocity);
        std::swap(angularVelocity, o.angularVelocity);
        std::swap(axis, o.axis);
        std::swap(maxAngulerVelocity, o.maxAngulerVelocity);
        std::swap(density, o.density);
        std::swap(m_bodyId, o.m_bodyId);
        std::swap(type, o.type);
        std::swap(forceAxis, o.forceAxis);
        std::swap(kinematic, o.kinematic);
    }

    void Body::release(JPH::BodyInterface& bodyInterface)
    {
        if (!m_bodyId.IsInvalid()) {
            bodyInterface.RemoveBody(m_bodyId);
            bodyInterface.DestroyBody(m_bodyId);
            m_bodyId = JPH::BodyID();
        }
    }

    void Body::create(
        physics::object_id objectId,
        uint32_t categoryMask,
        uint32_t collisionMask,
        JPH::PhysicsSystem& physicsSystem,
        const glm::vec3& scale)
    {
        invBaseRotation = glm::conjugate(baseRotation);

        if (type == BodyType::none) return;

        auto sz = scale * size;
        float radius = sz.x;
        float halfLength = sz.y;

        // Create shape based on type
        JPH::RefConst<JPH::Shape> shape;

        switch (type) {
        case BodyType::box: {
            // Jolt BoxShape takes half extents
            shape = new JPH::BoxShape(JPH::Vec3(sz.x, sz.y, sz.z));
            break;
        }
        case BodyType::sphere: {
            shape = new JPH::SphereShape(radius);
            break;
        }
        case BodyType::capsule: {
            // Jolt CapsuleShape: half-height of cylinder part, radius
            shape = new JPH::CapsuleShape(halfLength, radius);
            break;
        }
        case BodyType::cylinder: {
            // Jolt CylinderShape: half-height, radius
            shape = new JPH::CylinderShape(halfLength, radius);
            break;
        }
        default:
            return;
        }

        // Determine motion type and object layer
        JPH::EMotionType motionType = kinematic
            ? JPH::EMotionType::Kinematic
            : JPH::EMotionType::Dynamic;

        JPH::ObjectLayer objectLayer = toObjectLayer(categoryMask, kinematic, true);

        // Create body settings
        JPH::BodyCreationSettings settings(
            shape,
            JPH::RVec3::sZero(),  // Position will be set in updatePhysic
            JPH::Quat::sIdentity(),
            motionType,
            objectLayer);

        // Configure mass properties
        // Calculate volume for mass
        float volume = 1.0f;
        switch (type) {
        case BodyType::box:
            volume = sz.x * sz.y * sz.z * 8.0f; // Full box volume
            break;
        case BodyType::sphere:
            volume = (4.0f / 3.0f) * 3.14159f * radius * radius * radius;
            break;
        case BodyType::capsule:
        case BodyType::cylinder:
            volume = 3.14159f * radius * radius * halfLength * 2.0f;
            break;
        }

        settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
        settings.mMassPropertiesOverride.mMass = density * volume;

        // Set max angular velocity
        settings.mMaxAngularVelocity = maxAngulerVelocity;

        // Set friction and restitution defaults
        settings.mFriction = 0.5f;
        settings.mRestitution = 0.3f;

        // Pack user data: object_id and masks
        settings.mUserData = packUserData(objectId, categoryMask, collisionMask);

        // Create and add body
        JPH::BodyInterface& bodyInterface = physicsSystem.GetBodyInterface();
        m_bodyId = bodyInterface.CreateAndAddBody(settings, JPH::EActivation::Activate);
    }

    void Body::updatePhysic(
        JPH::BodyInterface& bodyInterface,
        const glm::vec3& nodePivot,
        const glm::vec3& nodePos,
        const glm::quat& nodeRot) const
    {
        setPhysicPosition(bodyInterface, nodePivot);
        setPhysicRotation(bodyInterface, nodeRot);
    }

    void Body::setPhysicPosition(JPH::BodyInterface& bodyInterface, const glm::vec3& pos) const
    {
        if (m_bodyId.IsInvalid()) return;
        bodyInterface.SetPosition(m_bodyId, toJoltR(pos), JPH::EActivation::DontActivate);
    }

    glm::vec3 Body::getPhysicPosition(const JPH::BodyInterface& bodyInterface) const
    {
        if (m_bodyId.IsInvalid()) return glm::vec3(0.f);
        return fromJolt(bodyInterface.GetPosition(m_bodyId));
    }

    void Body::setPhysicRotation(JPH::BodyInterface& bodyInterface, const glm::quat& rot) const
    {
        if (m_bodyId.IsInvalid()) return;
        bodyInterface.SetRotation(m_bodyId, toJolt(rot), JPH::EActivation::DontActivate);
    }

    glm::quat Body::getPhysicRotation(const JPH::BodyInterface& bodyInterface) const
    {
        if (m_bodyId.IsInvalid()) return glm::quat(1.f, 0.f, 0.f, 0.f);
        return fromJolt(bodyInterface.GetRotation(m_bodyId));
    }

    void Body::setLinearVelocity(JPH::BodyInterface& bodyInterface, const glm::vec3& vel) const
    {
        if (m_bodyId.IsInvalid()) return;
        bodyInterface.SetLinearVelocity(m_bodyId, toJolt(vel));
    }

    void Body::setAngularVelocity(JPH::BodyInterface& bodyInterface, const glm::vec3& vel) const
    {
        if (m_bodyId.IsInvalid()) return;
        bodyInterface.SetAngularVelocity(m_bodyId, toJolt(vel));
    }
}
