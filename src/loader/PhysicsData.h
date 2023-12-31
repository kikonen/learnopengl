#pragma once

#include "physics/Body.h"
#include "physics/Geom.h"

namespace loader {
    struct PhysicsData {
        bool enabled{ false };

        bool update{ false };
        physics::Body body;
        physics::Geom geom;
    };
}
