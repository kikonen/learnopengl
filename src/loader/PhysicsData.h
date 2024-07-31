#pragma once

#include <vector>

#include "physics/Category.h"
#include "physics/Body.h"
#include "physics/Geom.h"

namespace loader {
    struct PhysicsData {
        bool enabled{ false };

        bool update{ false };
        physics::Body body;
        physics::Geom geom;

        std::vector<physics::Category> categoryMask;
        std::vector<physics::Category> collisionMask;
    };
}
