#pragma once

#include <glm/glm.hpp>

#include "ki/RenderClock.h"

class Assets;
class Registry;

namespace kigl {
    class GLState;
}

namespace render {
    struct DebugContext;
}

//
// Context for doing updates, without rendering
//
struct UpdateViewContext final {
public:
    UpdateViewContext(
        const ki::RenderClock& clock,
        Registry* registry,
        int width,
        int height,
        const render::DebugContext* dbg);

public:
    const Assets& m_assets;

    const render::DebugContext* const m_dbg;

    const ki::RenderClock& m_clock;

    kigl::GLState& m_state;

    const glm::uvec2 m_resolution;

    Registry* const m_registry;
};
