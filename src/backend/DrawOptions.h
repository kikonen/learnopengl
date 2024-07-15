#pragma once

#include "ki/size.h"
#include "kigl/kigl.h"

#include "Lod.h"

namespace backend {
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsInstanced.xhtml
    struct DrawOptions {
        enum class Mode : std::underlying_type_t<std::byte> {
            none,
            points,
            // NOTE KI https://www.khronos.org/opengl/wiki/Primitive
            // patches == tessellation
            patches,
            triangles,
            triangle_strip,
            lines,
            line_srip
        };

        enum class Type : std::underlying_type_t<std::byte> {
            none,
            elements,
            arrays,
        };

        uint8_t m_patchVertices{ 3 };

        // - GL_TRIANGLES
        // - GL_TRIANGLE_STRIP
        // - GL_POINTS
        Mode m_mode : 3 = Mode::points;
        Type m_type : 2 = Type::none;

        bool m_solid : 1 {false};
        bool m_alpha : 1 {false};
        bool m_blend : 1 {false};

        bool m_renderBack : 1 {false};
        bool m_wireframe : 1 {false};

        bool m_gbuffer : 1 {false};

        uint8_t m_flags : 3 { 0 };

        uint8_t m_kindBits : 3 { 0 };

        GLuint toMode() const noexcept
        {
            switch (m_mode) {
            case Mode::points:
                return GL_POINTS;
            case Mode::patches:
                return GL_PATCHES;
            case Mode::triangles:
                return GL_TRIANGLES;
            case Mode::triangle_strip:
                return GL_TRIANGLE_STRIP;
            case Mode::lines:
                return GL_LINES;
            case Mode::line_srip:
                return GL_LINE_STRIP;
            }

            return 0;
        }

        inline bool isKind(
            uint8_t kindBits) const noexcept
        {
            return m_kindBits & kindBits;
        }

        inline bool isSameMultiDraw(
            const DrawOptions& o,
            bool forceSolid,
            bool forceWireframe) const noexcept
        {
            return m_renderBack == o.m_renderBack &&
                (forceWireframe ? true : m_wireframe == o.m_wireframe) &&
                (forceSolid ? true : m_blend == o.m_blend) &&
                m_mode == o.m_mode &&
                m_type == o.m_type;
        }

        // NOTE KI for MeshTypeKey
        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(m_blend, m_renderBack, m_wireframe, m_type, m_mode) <
                std::tie(o.m_blend, o.m_renderBack, o.m_wireframe, o.m_type, o.m_mode);
        }
    };
}
