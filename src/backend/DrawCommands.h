#pragma once

#include <glm/glm.hpp>

class FrameBuffer;
class Shader;

struct GLVertexArray;

namespace backend {
    struct DrawOptions;

    struct Flush {
    };

    struct ChangeBuffer {
        FrameBuffer* buffer;
    };

    struct DrawEntity {
        Shader* shader;
        GLVertexArray* vao;
        DrawOptions* drawOptions;
        bool allowBlend;
    };
}
