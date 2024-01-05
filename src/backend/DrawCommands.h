#pragma once

#include <glm/glm.hpp>

namespace render {
    class FrameBuffer;
}

class Program;

namespace kigl {
    struct GLVertexArray;
}

namespace backend {
    struct DrawOptions;

    struct Flush {
    };

    struct ChangeBuffer {
        render::FrameBuffer* buffer;
    };

    struct DrawEntity {
        Program* program;
        kigl::GLVertexArray* vao;
        DrawOptions* drawOptions;
        bool allowBlend;
    };
}
