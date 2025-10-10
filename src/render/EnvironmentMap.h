#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

struct PrepareContext;
class Program;

namespace render {
    class RenderContext;
    class CubeMap;

    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class EnvironmentMap
    {
    public:
        EnvironmentMap(std::string_view name)
            : m_name(name)
        {}

        ~EnvironmentMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        void prepareRT(
            const PrepareContext& ctx,
            int size);

        void bindTexture(
            kigl::GLState& state,
            int unitIndex);

        operator int() const { return m_cubeTexture; }

    public:
        int m_size{ 0 };
        std::string m_name;

        kigl::GLTextureHandle m_cubeTexture;
        kigl::GLTextureHandle m_flatTexture;

        GLuint m_hdriTextureID{ 0 };
    };
}
