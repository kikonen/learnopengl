#pragma once

#include <vector>
#include <string>

#include "ki/GL.h"

#include "kigl/GLTextureHandle.h"


class RenderContext;
class Assets;
class Registry;
class Program;
class GLState;

class CubeMap
{
public:
    CubeMap(bool empty);

    ~CubeMap();

    bool valid() { return m_textureID > 0; }

    void prepare(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx, int unitIndex);

private:
    void createEmpty();

    void createFaces();

    void createHdri(
        const Assets& assets,
        Registry* registry);

    void createIrradiance(
        const Assets& assets,
        Registry* registry);

    void renderCube(
        GLState& state,
        Program* program);

    operator int() const { return m_textureID; }

public:
    const bool m_empty;

    std::vector<std::string> m_faces;
    int m_size = 0;

    GLuint m_textureID = 0;

    GLenum m_internalFormat = GL_RGB8;
    bool m_hdri{ false };
    bool m_irradiance{ false };

    GLTextureHandle m_cubeTexture;

    CubeMap* m_envCubeMapRef;
};
