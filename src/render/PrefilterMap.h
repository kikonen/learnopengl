#pragma once

#include "kigl/kigl.h"

#include "kigl/GLTextureHandle.h"

class CubeMap;
class RenderContext;
class Assets;
class Registry;
class Program;
class GLState;

// NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
// - hdri is *linear*
class PrefilterMap
{
public:
    PrefilterMap() = default;
    ~PrefilterMap() = default;

    bool valid() { return m_cubeTexture.valid(); }

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx, int unitIndex);

    operator int() const { return m_cubeTexture; }

private:
    void render(
        GLState& state,
        Program* program,
        int cubeTextureID,
        int baseSize);

public:
    int m_size{ 0 };

    GLTextureHandle m_cubeTexture;

    int m_envCubeMapID{ 0 };
};
