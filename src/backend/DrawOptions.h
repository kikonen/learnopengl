#pragma once

#include "ki/size.h"
#include "kigl/kigl.h"

namespace backend {
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsInstanced.xhtml
    struct DrawOptions {
        enum class Type : std::underlying_type_t<std::byte> {
            none,
            elements,
            arrays,
        };

        Type type = Type::none;

        // - GL_TRIANGLES
        // - GL_TRIANGLE_STRIP
        // - GL_POINTS
        ki::size_t8 mode = GL_POINTS;

        bool renderBack : 1 {false};
        bool wireframe : 1 {false};
        bool blend : 1 {false};
        bool blendOIT : 1 {false};

        // NOTE KI STRONG assumption; instanced nodes are in same sequence
        // and first entry drawn is first one on that sequence
        bool instanced : 1 {false};

        bool tessellation : 1 {false};
        ki::size_t8 patchVertices{ 3 };

        // cont of indeces for indexed drawing
        // 0 for non indexed draw
        GLsizei indexCount{ 0 };
        GLsizei indexFirst{ 0 };

        ki::uint vertexOffset{ 0 };
        ki::uint indexOffset{ 0 };

        inline bool isSameDrawCommand(
            const DrawOptions& b,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            // NOTE KI multi/single material *CAN* go in same indirect draw
            // NOTE KI multiple "instanced" at once does not work
            return isSameMultiDraw(b, allowBlend, forceWireframe) &&
                vertexOffset == b.vertexOffset &&
                indexOffset == b.indexOffset &&
                (instanced == b.instanced ? !b.instanced : false);
        }

        inline bool isSameMultiDraw(
            const DrawOptions& b,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            return renderBack == b.renderBack &&
                (forceWireframe ? true : wireframe == b.wireframe) &&
                (allowBlend ? blend == b.blend : true) &&
                mode == b.mode &&
                type == b.type;
        }

        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(instanced, blend, renderBack, wireframe, type, mode, vertexOffset, indexOffset) <
                std::tie(instanced, o.blend, o.renderBack, o.wireframe, o.type, o.mode, o.vertexOffset, o.indexOffset);
        }
    };
}
