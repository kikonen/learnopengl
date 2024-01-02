#pragma once

#include "asset/Assets.h"

#include "kigl/GLTextureHandle.h"

class RenderContext;

//
// Prepare freetext-gl material
//
class FontAtlas
{
public:
    FontAtlas()
    {}

    void prepareRT(
        const Assets& assets);

    void bindTextures(GLState& state);
    void unbindTextures(GLState& state);

public:
    std::string m_fontName{ "fonts/Vera.ttf" };
    float m_fontSize{ 10.f };
    glm::uvec2 m_atlasSize{ 512, 512 };

    std::string m_text{ "" };

private:
    GLTextureHandle m_texture;
};
