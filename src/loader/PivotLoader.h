#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "model/PivotPoint.h"

#include "loader/document.h"

namespace loader {
    struct PivotLoader {
        model::PivotPoint load(const loader::DocNode& node) const;
    };
}
