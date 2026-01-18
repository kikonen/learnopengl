#pragma once

#include "loader/document.h"

#include "loader/PhysicsData.h"

namespace loader {
    struct PhysicsShapeValue {
        void loadShape(
            const loader::DocNode& node,
            loader::ShapeData& data) const;
    };
}
