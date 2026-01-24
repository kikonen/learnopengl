#pragma once

#include "DrawOptions.h"

#include "ki/size.h"

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct MultiDrawRange
    {
        const DrawOptions m_drawOptions;
        const ki::vao_id m_vaoId{ 0 };
        const ki::program_id m_programId{ 0 };
        const bool m_forceLineMode;
        const bool m_forceSolid;
    };
}
