#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

class CubeMap;
class RenderContext;
class Assets;
class Registry;
class Program;
class GLState;

namespace render {
    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class IrradianceMap
    {
    public:
        IrradianceMap() = default;
        ~IrradianceMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        void prepareRT(
            const Assets& assets,
            Registry* registry);

        void bindTexture(const RenderContext& ctx, int unitIndex);

        operator int() const { return m_cubeTexture; }

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;

        int m_envCubeMapID{ 0 };
    };
}
