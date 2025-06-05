#pragma once

#include <type_traits>

#include <glm/glm.hpp>

#include "ki/sid.h"

#include "component/LightType.h"

struct LightDefinition {
    LightType m_type{ LightType::none };

    ki::sid_t m_targetId{ 0 };

    float m_linear{ 0.f };
    float m_quadratic{ 0.f };

    float m_cutoffAngle{ 0.f };
    float m_outerCutoffAngle{ 0.f };

    glm::vec3 m_diffuse{ 0.5f, 0.5f, 0.5f };
    float m_intensity{ 1.f };
};
