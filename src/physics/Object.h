#pragma once

#include <ode/ode.h>

#include "ki/size.h"

#include "Body.h"
#include "Geom.h"

class NodeRegistry;

namespace physics {
    class PhysicsEngine;

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
        Object(Object&& b) noexcept;
        ~Object();

        inline bool ready() const { return m_geom.physicId || m_body.physicId; }

        void create(
            uint32_t entityIndex,
            dWorldID worldId,
            dSpaceID spaceId,
            NodeRegistry& nodeRegistry);

        void updateToPhysics(
            uint32_t entityIndex,
            ki::level_id& matrixLevel,
            NodeRegistry& nodeRegistry);

        void updateFromPhysics(
            uint32_t entityIndex,
            NodeRegistry& nodeRegistry) const;

        Body m_body{};
        Geom m_geom{};

    };
}
