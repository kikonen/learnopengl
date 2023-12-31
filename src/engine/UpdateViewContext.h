#pragma once

#include "ki/RenderClock.h"

#include "asset/Assets.h"

class Registry;

//
// Context for doing updates, without rendering
//
class UpdateViewContext {
public:
    UpdateViewContext(
        const ki::RenderClock& clock,
        const Assets& assets,
        Registry* registry,
        int width,
        int height);

public:
    const ki::RenderClock& m_clock;

    const Assets& m_assets;

    const glm::uvec2 m_resolution;

    Registry* const m_registry;
};
