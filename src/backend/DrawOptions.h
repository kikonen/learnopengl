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

        Type m_type = Type::none;

        // - GL_TRIANGLES
        // - GL_TRIANGLE_STRIP
        // - GL_POINTS
        ki::size_t8 m_mode = GL_POINTS;

        bool m_renderBack : 1 {false};
        bool m_wireframe : 1 {false};
        bool m_blend : 1 {false};
        bool m_blendOIT : 1 {false};

        // NOTE KI STRONG assumption; instanced nodes are in same sequence
        // and first entry drawn is first one on that sequence
        bool m_instanced : 1 {false};

        bool m_tessellation : 1 {false};
        ki::size_t8 m_patchVertices{ 3 };

        // cont of indeces for indexed drawing
        // 0 for non indexed draw
        GLsizei m_indexCount{ 0 };
        GLsizei m_indexFirst{ 0 };

        ki::uint m_vertexOffset{ 0 };
        ki::uint m_indexOffset{ 0 };

        inline bool isSameDrawCommand(
            const DrawOptions& b,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            // NOTE KI multi/single material *CAN* go in same indirect draw
            // NOTE KI multiple "instanced" at once does not work
            return isSameMultiDraw(b, allowBlend, forceWireframe) &&
                m_vertexOffset == b.m_vertexOffset &&
                m_indexOffset == b.m_indexOffset &&
                (m_instanced == b.m_instanced ? !b.m_instanced : false);
        }

        inline bool isSameMultiDraw(
            const DrawOptions& b,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            return m_renderBack == b.m_renderBack &&
                (forceWireframe ? true : m_wireframe == b.m_wireframe) &&
                (allowBlend ? m_blend == b.m_blend : true) &&
                m_mode == b.m_mode &&
                m_type == b.m_type;
        }

        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(m_instanced, m_blend, m_renderBack, m_wireframe, m_type, m_mode, m_vertexOffset, m_indexOffset) <
                std::tie(o.m_instanced, o.m_blend, o.m_renderBack, o.m_wireframe, o.m_type, o.m_mode, o.m_vertexOffset, o.m_indexOffset);
        }
    };
}
