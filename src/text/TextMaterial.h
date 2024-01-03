#pragma once

#include "asset/CustomMaterial.h"

#include "text/FontAtlas.h"

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
    text::FontAtlas m_atlas;

    std::string m_text{ "" };
};
