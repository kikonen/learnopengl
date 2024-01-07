#include "TextMaterial.h"

#include "render/RenderContext.h"

#include "text/FontAtlas.h"

#include "registry/Registry.h"
#include "registry/FontRegistry.h"

void TextMaterial::prepareRT(
    const PrepareContext& ctx)
{
}

void TextMaterial::updateRT(const RenderContext& ctx)
{
    if (!m_dirty) return;

    //updateBuffer(ctx);
    m_dirty = false;
}

void TextMaterial::bindTextures(const RenderContext& ctx)
{
    ctx.m_registry->m_fontRegistry->bindFont(ctx.m_state, m_fontId);
}

void TextMaterial::unbindTextures(const RenderContext& ctx)
{
    ctx.m_registry->m_fontRegistry->unbindFont(ctx.m_state, m_fontId);
}
