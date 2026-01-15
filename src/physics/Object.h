#pragma once

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "ki/size.h"

#include "Body.h"
#include "Geom.h"

class NodeRegistry;

namespace JPH {
    class PhysicsSystem;
    class BodyInterface;
}

namespace physics {
    class PhysicsSystem;

    // How can I make my actor capsule(capped cylinder) stay upright ?
    // Manually reset the orientation with each frame, use an angular motor,
    // apply torques, use a sphere instead, or (quoting Jon Watte) instead use
    // "a hovering sphere with a ray to 'sense' where the ground is and apply spring
    // forces to keep it at a constant height above ground." See 1, 2, 3 for details.
    // A HOWTO is being set - up over here : HOWTO upright capsule
    //
    // https://ode.org/wiki/index.php/HOWTO_upright_capsule
    //
    // https://www.ode.org/old_list_archives/2006-June/019114.html
    // The locked orientation via setting the quaternion in per-frame (which
    // you've got to do per time-step, mind you, you can't get away with
    // setting it outside of your per - timestep looped code) works really,
    // really well, but you've got to use dBodySetAngularVelocity and
    // dBodySetTorque to zero out any torque forces.If you don't, they pile
    // up, and you get some really strange behavior(especially when touching
    // other objects).
    struct Object {
        Object();
        Object(Object& o) = delete;
        Object(const Object& o) = delete;
        Object(Object&& o) noexcept
        {
            swap(o);
        }

        ~Object();

        Object& operator=(Object& o) = delete;
        Object& operator=(Object&& o) noexcept
        {
            Object tmp(std::move(o));
            swap(tmp);
            return *this;
        }

        void swap(Object& o) noexcept;

        inline bool ready() const {
            return m_body.hasPhysicsBody() || m_geom.hasPhysicsBody();
        }

        void release(JPH::BodyInterface& bodyInterface);

        void create(
            physics::object_id objectId,
            uint32_t entityIndex,
            JPH::PhysicsSystem& physicsSystem,
            NodeRegistry& nodeRegistry);

        bool updateToPhysics(
            uint32_t entityIndex,
            ki::level_id& matrixLevel,
            JPH::BodyInterface& bodyInterface,
            NodeRegistry& nodeRegistry);

        void updateFromPhysics(
            uint32_t entityIndex,
            const JPH::BodyInterface& bodyInterface,
            NodeRegistry& nodeRegistry) const;

    public:
        Body m_body{};
        Geom m_geom{};
    };
}
