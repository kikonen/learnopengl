#pragma once

#include <string>

#include <glm/glm.hpp>

#include "physics/Object.h"

namespace loader {
    struct PhysicsData {
        bool enabled{ false };

        bool update{ false };
        physics::Body body;
        physics::Geom geom;
    };
}
