#pragma once

#include <glm/glm.hpp>

#include "BaseUUID.h"

class Light;

namespace loader {
    enum class LightType : std::underlying_type_t<std::byte> {
        none,
        directional,
        point,
        spot
    };

    struct LightData {
        bool enabled{ false };
        LightType type{ LightType::none };

        BaseUUID targetIdBase;

        float linear{ 0.f };
        float quadratic{ 0.f };

        float cutoffAngle{ 0.f };
        float outerCutoffAngle{ 0.f };

        glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };
        float intensity{ 1.f };
    };
}
