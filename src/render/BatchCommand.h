#pragma once

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Program;
class MaterialVBO;


struct BatchCommand {
    const Program* m_program{ nullptr };
    //MeshType* m_type{ nullptr };

    const kigl::GLVertexArray* m_vao{ nullptr };
    const backend::DrawOptions* m_drawOptions{ nullptr };

    //const MaterialVBO* m_materialVBO{ nullptr };

    int m_index = 0;
    int m_drawCount = 0;
    int m_instancedCount = 1;
};
