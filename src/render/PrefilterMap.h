#pragma once

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

struct PrepareContext;

class Program;
class CubeMap;

namespace render {
    class RenderContext;

    // NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
    // - hdri is *linear*
    class PrefilterMap
    {
    public:
        PrefilterMap() = default;
        ~PrefilterMap() = default;

        bool valid() { return m_cubeTexture.valid(); }

        void prepareRT(
            const PrepareContext& ctx);

        void bindTexture(
            kigl::GLState& state,
            int unitIndex);

        operator int() const { return m_cubeTexture; }

    private:
        void render(
            Program* program,
            int cubeTextureID,
            int baseSize);

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;
        kigl::GLTextureHandle m_flatTexture;

        int m_envCubeMapID{ 0 };
    };
}
