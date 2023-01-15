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
        bool blend = false;

        // NOTE KI STRONG assumption; instanced nodes are in same sequence
        // and first entry drawn is first one on that sequence
        bool instanced = false;

        // cont of indeces for indexed drawing
        // 0 for non indexed draw
        GLsizei indexCount = 0;
        GLsizei indexFirst = 0;

        int vertexOffset = 0;
        int indexOffset = 0;

        int materialOffset = 0;
        bool singleMaterial = false;

        float volumeRadiusFlex = 1.f;

        inline bool isSameDrawCommand(const DrawOptions& b, bool useBlend) const {
            // NOTE KI multi/single material *CAN* go in same indirect draw
            return isSameMultiDraw(b, useBlend) &&
                vertexOffset == b.vertexOffset &&
                indexOffset == b.indexOffset &&
                instanced == b.instanced;
        }

        inline bool isSameMultiDraw(const DrawOptions& b, bool useBlend) const {
            return renderBack == b.renderBack &&
                wireframe == b.wireframe &&
                (useBlend ? blend == b.blend : true) &&
                mode == b.mode &&
                type == b.type;
        }

        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(blend, renderBack, wireframe, type, mode, singleMaterial, vertexOffset, indexOffset, materialOffset) <
                std::tie(o.blend, o.renderBack, o.wireframe, o.type, o.mode, o.singleMaterial, o.vertexOffset, o.indexOffset, o.materialOffset);
        }
    };
}
