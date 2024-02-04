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
        uint8_t m_mode = GL_POINTS;

        bool m_renderBack : 1 {false};
        bool m_wireframe : 1 {false};
        bool m_blend : 1 {false};
        bool m_blendOIT : 1 {false};

        bool m_tessellation : 1 {false};
        uint8_t m_patchVertices{ 3 };

        // count of indeces for indexed drawing
        // 0 for non indexed draw
        GLsizei m_indexCount{ 0 };
        GLsizei m_indexFirst{ 0 };

        uint32_t m_vertexOffset{ 0 };
        uint32_t m_indexOffset{ 0 };

        inline bool isSameDrawCommand(
            const DrawOptions& b,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            // NOTE KI multi/single material *CAN* go in same indirect draw
            return isSameMultiDraw(b, allowBlend, forceWireframe) &&
                m_vertexOffset == b.m_vertexOffset &&
                m_indexOffset == b.m_indexOffset;
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
                m_type == b.m_type &&
                m_tessellation == b.m_tessellation;
        }

        // NOTE KI for MeshTypeKey/MeshTypeComparator
        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(m_blend, m_renderBack, m_wireframe, m_type, m_mode, m_vertexOffset, m_indexOffset) <
                std::tie(o.m_blend, o.m_renderBack, o.m_wireframe, o.m_type, o.m_mode, o.m_vertexOffset, o.m_indexOffset);
        }
    };
}
