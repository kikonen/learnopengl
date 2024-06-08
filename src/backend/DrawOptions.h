#pragma once

#include "ki/size.h"
#include "kigl/kigl.h"

#include "Lod.h"

namespace backend {
    // https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsInstanced.xhtml
    struct DrawOptions {
        enum class Type : std::underlying_type_t<std::byte> {
            none,
            elements,
            arrays,
        };

        // - GL_TRIANGLES
        // - GL_TRIANGLE_STRIP
        // - GL_POINTS
        uint8_t m_mode = GL_POINTS;
        uint8_t m_patchVertices{ 3 };

        Type m_type : 2 = Type::none;

        bool m_alpha : 1 {false};
        bool m_blend : 1 {false};

        bool m_gbuffer : 1 {false};
        bool m_blendOIT : 1 {false};

        bool m_renderBack : 1 {false};
        bool m_wireframe : 1 {false};

        bool m_tessellation : 1 {false};

        unsigned int m_kindBits{ 0 };

        inline bool isKind(
            unsigned int kindBits) const noexcept
        {
            return m_kindBits & kindBits;
        }

        inline bool isSameDrawCommand(
            const DrawOptions& o,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            // NOTE KI multi/single material *CAN* go in same indirect draw
            return isSameMultiDraw(o, allowBlend, forceWireframe);
        }

        inline bool isSameMultiDraw(
            const DrawOptions& o,
            bool forceWireframe,
            bool allowBlend) const noexcept
        {
            return m_renderBack == o.m_renderBack &&
                (forceWireframe ? true : m_wireframe == o.m_wireframe) &&
                (allowBlend ? m_blend == o.m_blend : true) &&
                m_mode == o.m_mode &&
                m_type == o.m_type &&
                m_tessellation == o.m_tessellation;
        }

        // NOTE KI for MeshTypeKey
        inline bool operator<(const DrawOptions& o) const noexcept {
            return std::tie(m_blend, m_renderBack, m_wireframe, m_type, m_mode) <
                std::tie(o.m_blend, o.m_renderBack, o.m_wireframe, o.m_type, o.m_mode);
        }
    };
}
