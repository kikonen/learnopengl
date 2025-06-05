#pragma once

#include <type_traits>

#include <glm/glm.hpp>

#include "BaseId.h"

#include "component/LightType.h"

struct LightDefinition;

namespace loader {
    struct LightData {
        bool enabled{ false };
        LightType type{ LightType::none };

        BaseId targetBaseId;

        float linear{ 0.f };
        float quadratic{ 0.f };

        float cutoffAngle{ 0.f };
        float outerCutoffAngle{ 0.f };

        glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };
        float intensity{ 1.f };
    };
}
