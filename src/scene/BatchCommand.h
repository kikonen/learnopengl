#pragma once

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Shader;
class MaterialVBO;


struct BatchCommand {
    const Shader* m_shader{ nullptr };
    //MeshType* m_type{ nullptr };

    const GLVertexArray* m_vao{ nullptr };
    const backend::DrawOptions* m_drawOptions{ nullptr };

    const MaterialVBO* m_materialVBO{ nullptr };

    int m_index = 0;
    int m_drawCount = 0;
    int m_instancedCount = 1;
};
