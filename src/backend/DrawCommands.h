#pragma once

#include <glm/glm.hpp>

class FrameBuffer;
class Program;

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawOptions;

    struct Flush {
    };

    struct ChangeBuffer {
        FrameBuffer* buffer;
    };

    struct DrawEntity {
        Program* program;
        kigl::GLVertexArray* vao;
        DrawOptions* drawOptions;
        bool allowBlend;
    };
}
