#pragma once

#include <string>

#include <glm/glm.hpp>

#include "animation/RigSocket.h"

#include "TransformData.h"

namespace loader {
    struct SocketData {
        bool enabled{ false };

        std::string name;
        std::string joint;
        animation::SocketRole role{ animation::SocketRole::general };

        TransformData offset;
    };
}
