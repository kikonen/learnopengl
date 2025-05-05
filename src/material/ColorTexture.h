#pragma once

#include <glm/glm.hpp>

#include "Texture.h"

// Single pixel, single color texture
class ColorTexture : public Texture {
public:
    static const ColorTexture& getWhiteRGBA();
    static const ColorTexture& getWhiteRGB();
    static const ColorTexture& getWhiteR();

    static const ColorTexture& getBlackRGBA();

    ColorTexture(
        std::string_view name,
        glm::vec4 color,
        GLenum internalFormat,
        bool usePrepare);

    ~ColorTexture();

    void release() override;
    void prepare() override;

private:
    glm::vec4 m_color;
};
