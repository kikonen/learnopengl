#pragma once

#include "asset/CustomMaterial.h"

#include "text/size.h"

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
        const PrepareContext& ctx) override;

    virtual void updateRT(
        const RenderContext& ctx) override;

    virtual void bindTextures(const RenderContext& ctx) override;
    virtual void unbindTextures(const RenderContext& ctx) override;

public:
    text::font_id m_fontId{ 0 };

    std::string m_text{ "" };
};
