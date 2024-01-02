#include "TextMaterial.h"

#include "render/RenderContext.h"


void TextMaterial::prepareRT(
    const Assets& assets,
    Registry* registry)
{
    m_atlas.prepareRT(assets);
}

void TextMaterial::updateRT(const RenderContext& ctx)
{
    if (!m_dirty) return;

    //updateBuffer(ctx);
    m_dirty = false;
}

void TextMaterial::bindTextures(const RenderContext& ctx)
{
    m_atlas.bindTextures(ctx.m_state);
}

void TextMaterial::unbindTextures(const RenderContext& ctx)
{
    m_atlas.unbindTextures(ctx.m_state);
}
