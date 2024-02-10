#pragma once

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Program;

namespace backend {
    struct Lod;
}

namespace render {
    struct BatchCommand {
        const Program* m_program{ nullptr };

        const kigl::GLVertexArray* m_vao{ nullptr };
        backend::DrawOptions m_drawOptions;

        int m_baseIndex{ 0 };
        int m_instanceCount{ 0 };

        const backend::Lod* m_lod{ nullptr };
    };
}
