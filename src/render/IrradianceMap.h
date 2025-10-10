#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

class CubeMap;

struct PrepareContext;

class Program;

namespace render {
    class RenderContext;

    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class IrradianceMap
    {
    public:
        IrradianceMap() = default;
        ~IrradianceMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        void prepareRT(
            const PrepareContext& ctx);

        void bindTexture(
            kigl::GLState& state,
            int unitIndex);

        operator int() const { return m_cubeTexture; }

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;
        kigl::GLTextureHandle m_flatTexture;

        int m_envCubeMapID{ 0 };
    };
}
