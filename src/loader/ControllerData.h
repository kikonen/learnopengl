#pragma once

#include <glm/glm.hpp>

#include "loader/BaseId.h"

namespace loader {
    enum class ControllerType : std::underlying_type_t<std::byte> {
        none,
        pawn,
        camera_zoom,
    };

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
