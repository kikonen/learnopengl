#pragma once

#include <type_traits>

#include <glm/glm.hpp>

#include "component/ControllerDefinition.h"

#include "loader/BaseId.h"

namespace loader {
    struct ControllerData {
        bool enabled{ false };
        ControllerType type{ ControllerType::none };

        int mode{ 0 };
        float speed{ 0.f };

        BaseId targetBaseId;

        glm::vec3 direction{ 0.f, 0.f, 1.f };
        float distance{ 0.f };
    };
}
