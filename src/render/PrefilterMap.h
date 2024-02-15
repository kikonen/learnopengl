#pragma once

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

struct PrepareContext;
class RenderContext;

class CubeMap;
class Program;

namespace render {
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

        void bindTexture(const RenderContext& ctx, int unitIndex);

        operator int() const { return m_cubeTexture; }

    private:
        void render(
            Program* program,
            int cubeTextureID,
            int baseSize);

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_cubeTexture;

        int m_envCubeMapID{ 0 };
    };
}
