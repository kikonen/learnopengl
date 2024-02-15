#pragma once

#include <ode/ode.h>

#include "ki/size.h"

#include "pool/NodeHandle.h"

#include "size.h"
#include "Body.h"
#include "Geom.h"

namespace physics {
    class PhysicsEngine;

    struct Object {
        Object();
        Object(Object&& b) noexcept;
        ~Object();

        inline bool ready() const { return m_geomId || m_bodyId; }

        void prepare(
            dWorldID worldId,
            dSpaceID spaceId);

        void updateToPhysics(bool force);
        void updateFromPhysics() const;

        physics::physics_id m_id{ 0 };

        Body m_body{};
        Geom m_geom{};

        dMass m_mass;
        dBodyID m_bodyId{ nullptr };
        dGeomID m_geomId{ nullptr };

        pool::NodeHandle m_nodeHandle{};

        ki::level_id m_matrixLevel{ 0 };
        bool m_update : 1{ false };
    };
}
