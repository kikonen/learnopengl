#pragma once

#include <type_traits>

#include "ki/size.h"
#include "kigl/kigl.h"

#include "Lod.h"

#include "render/size.h"

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

        bool m_renderBack : 1 {false};
        bool m_lineMode : 1 {false};
        // GL_CCW vs GL_CW (GL_CCW == normal)
        bool m_reverseFrontFace : 1 {false};
        bool m_noDepth : 1 {false};

        bool m_gbuffer : 1 {false};

        bool m_clip : 1 {false};

        uint8_t m_flags : 3 { 0 };

        uint8_t m_kindBits : 3 { render::KIND_SOLID };

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

        inline bool isSolid() const noexcept
        {
            return isKind(render::KIND_SOLID);
        }

        inline bool isAlpha() const noexcept
        {
            return isKind(render::KIND_ALPHA);
        }

        inline bool isBlend() const noexcept
        {
            return isKind(render::KIND_BLEND);
        }

        inline bool isSameMultiDraw(
            const DrawOptions& o) const noexcept
        {
            // NOTE KI KIND_SOLID & KIND_ALPHA can be in same multidraw
            return m_renderBack == o.m_renderBack &&
                m_lineMode == o.m_lineMode &&
                m_reverseFrontFace == o.m_reverseFrontFace &&
                m_noDepth == o.m_noDepth &&
                m_clip == o.m_clip &&
                m_mode == o.m_mode &&
                m_type == o.m_type &&
                isBlend() == o.isBlend();
        }

        // NOTE KI for NodeTypeKey
        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(m_kindBits, m_renderBack, m_clip, m_lineMode, m_noDepth, m_reverseFrontFace, m_type, m_mode) <
                std::tie(o.m_kindBits, o.m_renderBack, o.m_clip, o.m_lineMode, o.m_noDepth, o.m_reverseFrontFace, o.m_type, o.m_mode);
        }

        // NOTE KI *strict* equality; MultiDraw is lesser than strict
        inline bool operator==(const DrawOptions& o) const noexcept {
            return std::tie(m_kindBits, m_renderBack, m_clip, m_lineMode, m_noDepth, m_reverseFrontFace, m_type, m_mode) ==
                std::tie(o.m_kindBits, o.m_renderBack, o.m_clip, o.m_lineMode, o.m_noDepth, o.m_reverseFrontFace, o.m_type, o.m_mode);
        }
    };
}
