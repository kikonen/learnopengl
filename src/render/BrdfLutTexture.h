#pragma once

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

struct PrepareContext;
class Program;

namespace render {
    class RenderContext;

    // https://learnopengl.com/PBR/IBL/Specular-IBL
    class BrdfLutTexture
    {
    public:
        BrdfLutTexture() = default;
        ~BrdfLutTexture() = default;

        bool valid() { return m_texture.valid(); }

        void prepareRT(
            const PrepareContext& ctx);

        void bindTexture(
            kigl::GLState& state,
            int unitIndex);

        operator int() const { return m_texture; }

    private:
        void render(
            Program* program,
            int cubeTextureID,
            int baseSize);

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_texture;
    };
}
