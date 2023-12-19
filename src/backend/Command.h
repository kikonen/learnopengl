#pragma once

#include "DrawCommands.h"

namespace backend {
    enum class RenderType : std::underlying_type_t<std::byte> {
        buffer,
        flush,
        draw
    };

    //
    // Render command
    //
    struct Command {
        RenderType m_type;
        union {
            Flush m_flush;
            ChangeBuffer m_changeBuffer;
            DrawEntity m_draw;
        };
    };
}
