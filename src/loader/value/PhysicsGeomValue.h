#pragma once

#include "physics/Geom.h"

#include "loader/document.h"

namespace loader {
    struct PhysicsGeomValue {
        void loadGeom(
            const loader::DocNode& node,
            physics::Geom& data) const;
    };
}
