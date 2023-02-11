#pragma once

#include <glm/glm.hpp>

class FrameBuffer;
class Program;

struct GLVertexArray;

namespace backend {
    struct DrawOptions;

    struct Flush {
    };

    struct ChangeBuffer {
        FrameBuffer* buffer;
    };

    struct DrawEntity {
        Program* program;
        GLVertexArray* vao;
        DrawOptions* drawOptions;
        bool allowBlend;
    };
}
