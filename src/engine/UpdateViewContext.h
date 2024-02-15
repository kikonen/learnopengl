#pragma once

#include <glm/glm.hpp>

#include "ki/RenderClock.h"

class Assets;
class Registry;

//
// Context for doing updates, without rendering
//
struct UpdateViewContext final {
public:
    UpdateViewContext(
        const ki::RenderClock& clock,
        Registry* registry,
        int width,
        int height);

public:
    const Assets& m_assets;

    const ki::RenderClock& m_clock;

    const glm::uvec2 m_resolution;

    Registry* const m_registry;
};
