#pragma once

#include "loader/document.h"

#include "loader/PhysicsData.h"

namespace loader {
    struct PhysicsGeomValue {
        void loadGeom(
            const loader::DocNode& node,
            loader::GeomData& data) const;
    };
}
