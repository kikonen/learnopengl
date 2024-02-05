#pragma once

#include <glm/glm.hpp>

#include "ki/RenderClock.h"

//
// Context for doing updates, without rendering
//
struct UpdateViewContext final {
public:
    UpdateViewContext(
        const ki::RenderClock& clock,
        int width,
        int height);

public:
    const ki::RenderClock& m_clock;

    const glm::uvec2 m_resolution;
};
