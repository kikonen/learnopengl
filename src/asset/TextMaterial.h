#pragma once

#include "CustomMaterial.h"

//
// Prepare freetext-gl material
//
class TextMaterial : public CustomMaterial
{
public:
    TextMaterial()
    : CustomMaterial("text", false)
    {}

    virtual void prepareRT(
        const Assets& assets,
        Registry* registry) override;

    virtual void updateRT(
        const RenderContext& ctx) override;

    virtual void bindTextures(const RenderContext& ctx) override;
    virtual void unbindTextures(const RenderContext& ctx) override;

public:
    std::string m_fontName{ "fonts/Vera.ttf" };
    float m_fontSize{ 10.f };
    glm::uvec2 m_atlasSize{ 512, 512 };

    std::string m_text{ "" };
};
