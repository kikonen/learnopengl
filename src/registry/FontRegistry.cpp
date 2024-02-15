#include "FontRegistry.h"

#include "engine/UpdateContext.h"

#include "kigl/GLState.h"

FontRegistry& FontRegistry::get() noexcept
{
    static FontRegistry s_registry;
    return s_registry;
}

FontRegistry::FontRegistry()
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
        font.prepare();
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
    text::font_id id)
{
    auto* font = getFont(id);
    //if (font) {
    //    font->bindTextures();
    //}
    return font;
}

bool FontRegistry::unbindFont(
    text::font_id id)
{
    auto* font = getFont(id);
    //if (font) {
    //    font->unbindTextures();
    //}
    return font;
}

text::font_id FontRegistry::registerFont(
    const std::string& name)
{
    std::unique_lock lock(m_lock);

    auto& font = m_fonts.emplace_back<text::FontAtlas>({});
    font.m_name = name;
    font.m_id = static_cast<text::font_id>(m_fonts.size());

    return font.m_id;
}
