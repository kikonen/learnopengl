#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

class RenderContext;
class Assets;
class Registry;
class Program;

namespace render {
    class CubeMap;

    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class EnvironmentMap
    {
    public:
        EnvironmentMap() = default;
        ~EnvironmentMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        void prepareRT(
            const Assets& assets,
            Registry* registry,
            int size);

        void bindTexture(const RenderContext& ctx, int unitIndex);

        operator int() const { return m_cubeTexture; }

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;

        GLuint m_hdriTextureID{ 0 };
    };
}
