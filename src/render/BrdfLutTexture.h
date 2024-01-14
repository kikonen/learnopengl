#pragma once

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

struct PrepareContext;
class RenderContext;

class Program;

namespace kigl {
    class GLState;
}

namespace render {
    // https://learnopengl.com/PBR/IBL/Specular-IBL
    class BrdfLutTexture
    {
    public:
        BrdfLutTexture() = default;
        ~BrdfLutTexture() = default;

        bool valid() { return m_texture.valid(); }

        void prepareRT(
            const PrepareContext& ctx);

        void bindTexture(const RenderContext& ctx, int unitIndex);

        operator int() const { return m_texture; }

    private:
        void render(
            kigl::GLState& state,
            Program* program,
            int cubeTextureID,
            int baseSize);

    public:
        int m_size{ 0 };

        kigl::GLTextureHandle m_texture;
    };
}
