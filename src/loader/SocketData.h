#pragma once

#include <string>

#include <glm/glm.hpp>

#include "TransformData.h"

namespace loader {
    struct SocketData {
        bool enabled{ false };

        std::string name;
        std::string joint;

        TransformData offset;
    };
}
