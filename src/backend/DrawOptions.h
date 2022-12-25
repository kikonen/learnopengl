#pragma once

#include "ki/GL.h"

namespace backend {
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsInstanced.xhtml
    struct DrawOptions {
        enum class Type {
            none,
            elements,
            arrays,
        };

        Type type = Type::none;

        // - GL_TRIANGLES
        // - GL_TRIANGLE_STRIP
        // - GL_POINTS
        GLenum mode = GL_POINTS;

        bool renderBack = false;
        bool wireframe = false;

        // cont of indeces for indexed drawing
        // 0 for non indexed draw
        GLsizei indexCount = 0;
        GLsizei indexFirst = 0;

        int vertexOffset = 0;
        int indexOffset = 0;

        int materialOffset = 0;
        int materialCount = 0;

        bool isSameDrawCommand(const DrawOptions& b) const {
            return isSameMultiDraw(b) &&
                vertexOffset == b.vertexOffset &&
                indexOffset == b.indexOffset &&
                materialCount == b.materialCount;
        }

        bool isSameMultiDraw(const DrawOptions& b) const {
            return renderBack == b.renderBack &&
                wireframe == b.wireframe &&
                mode == b.mode &&
                type == b.type;
        }

        bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(renderBack, wireframe, type, mode, materialCount, vertexOffset, indexOffset, materialOffset) <
                std::tie(o.renderBack, o.wireframe, o.type, o.mode, o.materialCount, o.vertexOffset, o.indexOffset, o.materialOffset);
        }
    };
}
