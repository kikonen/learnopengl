#include "FontRegistry.h"

#include "engine/UpdateContext.h"

#include "kigl/GLState.h"

FontRegistry::FontRegistry(
    const Assets& assets)
    : m_assets(assets)
{}

FontRegistry::~FontRegistry()
{
    m_fonts.clear();
}

void FontRegistry::prepareRT()
{
}

void FontRegistry::updateRT(const UpdateContext& ctx)
{
    std::shared_lock lock(m_lock);

    for (auto& font : m_fonts) {
        font.prepare(ctx.m_assets);
        font.update();
    }
}

text::FontAtlas* FontRegistry::modifyFont(text::font_id id)
{
    if (id < 1) return nullptr;

    std::shared_lock lock(m_lock);
    assert(id > 0 && id <= m_fonts.size());

    return &m_fonts[id - 1];
}

bool FontRegistry::bindFont(
    kigl::GLState& state,
    text::font_id id)
{
    auto* font = getFont(id);
    if (font) {
        font->bindTextures(state);
    }
    return font;
}

bool FontRegistry::unbindFont(
    kigl::GLState& state,
    text::font_id id)
{
    auto* font = getFont(id);
    if (font) {
        font->unbindTextures(state);
    }
    return font;
}

text::font_id FontRegistry::registerFont(
    const std::string& name)
{
    std::unique_lock lock(m_lock);

    auto& font = m_fonts.emplace_back<text::FontAtlas>({});
    font.m_name = name;
    font.m_id = static_cast<ki::type_id>(m_fonts.size());

    return font.m_id;
}
