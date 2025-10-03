#pragma once

#include <glm/glm.hpp>

#include "engine/BaseContext.h"

//
// Context for doing updates, without rendering
//
struct UpdateViewContext final : BaseContext {
public:
    UpdateViewContext(
        Engine& engine,
        int width,
        int height);

public:
    const glm::uvec2 m_resolution;
};
