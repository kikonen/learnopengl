#pragma once

#include <glm/glm.hpp>

#include "Texture.h"

// Single pixel, single color texture
class ColorTexture : public Texture {
public:
    static util::Ref<ColorTexture> getWhiteRGBA(bool prepare);
    static util::Ref<ColorTexture> getWhiteRGB(bool prepare);
    static util::Ref<ColorTexture> getWhiteR(bool prepare);

    static util::Ref<ColorTexture> getBlackRGBA(bool prepare);

    static util::Ref<ColorTexture> getFlatNormalRGBA(bool prepare);

    ColorTexture(
        std::string_view name,
        glm::vec4 color,
        GLenum internalFormat);

    ~ColorTexture();

    void release() override;
    void prepareSingle() override;

    void prepareArray(
        const util::Ref<ArrayTexture>& arr,
        uint32_t layer) override;

private:
    glm::vec4 m_color;

    GLenum m_pixelFormat{ 0 };
};
