#pragma once

#include <vector>
#include <string>

#include "kigl/kigl.h"

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

    bool valid() { return m_cubeTexture > 0; }

    void prepareRT(
        const Assets& assets,
        Registry* registry);

    void bindTexture(const RenderContext& ctx, int unitIndex);

    operator int() const { return m_cubeTexture; }

private:
    void createEmpty();

    void createFaces();

public:
    const bool m_empty;

    std::vector<std::string> m_faces;
    int m_size = 0;

    GLenum m_internalFormat = GL_RGB8;

    GLTextureHandle m_cubeTexture;
};
