#pragma once

#include <glm/glm.hpp>

class FrameBuffer;
class Shader;

namespace backend {
    struct Flush {
    };

    struct ChangeBuffer {
        FrameBuffer* buffer;
    };

    struct Draw {
        Shader* m_shader;

        glm::mat4& m_modelMatrix;
        glm::mat3& m_normalMatrix;

        int m_objectID;

        bool wireframe;
        bool renderBack;
    };
}
