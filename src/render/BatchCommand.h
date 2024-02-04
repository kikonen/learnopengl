#pragma once

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Program;


namespace render {
    struct BatchCommand {
        const Program* m_program{ nullptr };

        const kigl::GLVertexArray* m_vao{ nullptr };
        backend::DrawOptions m_drawOptions;

        int m_index{ 0 };
        int m_instanceCount{ 0 };
    };
}
