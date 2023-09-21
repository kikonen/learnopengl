#pragma once

#include <vector>
#include <string>

#include "ki/GL.h"

#include "kigl/GLTextureHandle.h"

class CubeMap;
class RenderContext;
class Assets;
class Registry;
class Program;
class GLState;

// NOTE KI https://forums.cgsociety.org/t/gamma-and-hdri/959636
// - hdri is *linear*
class EnvironmentMap
{
public:
    EnvironmentMap() = default;
    ~EnvironmentMap() = default;

    bool valid() { return m_cubeTexture.valid(); }

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx, int unitIndex);

    operator int() const { return m_cubeTexture; }

public:
    int m_size{ 0 };

    GLTextureHandle m_cubeTexture;

    GLuint m_hdriTextureID;
};
