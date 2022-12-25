#pragma once

#include <glm/glm.hpp>

#include "kigl/GLVertexArray.h"

#include "backend/DrawOptions.h"


class Shader;
class MaterialVBO;


struct BatchCommand {
    Shader* m_shader{ nullptr };
    //MeshType* m_type{ nullptr };

    GLVertexArray* m_vao{ nullptr };
    backend::DrawOptions m_drawOptions;

    MaterialVBO* m_materialVBO{ nullptr };
};
