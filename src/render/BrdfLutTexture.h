#pragma once

#include "ki/GL.h"

#include "kigl/GLTextureHandle.h"

class RenderContext;
class Assets;
class Registry;
class Program;
class GLState;

// https://learnopengl.com/PBR/IBL/Specular-IBL
class BrdfLutTexture
{
public:
    BrdfLutTexture() = default;
    ~BrdfLutTexture() = default;

    bool valid() { return m_texture.valid(); }

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx, int unitIndex);

    operator int() const { return m_texture; }

private:
    void render(
        GLState& state,
        Program* program,
        int cubeTextureID,
        int baseSize);

public:
    int m_size{ 0 };

    GLTextureHandle m_texture;
};