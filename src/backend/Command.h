#pragma once

#include "DrawCommands.h"

namespace backend {
    enum class RenderType {
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
            Draw m_draw;
        };
    };
}
