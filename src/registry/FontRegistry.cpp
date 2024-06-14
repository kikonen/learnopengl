#include "FontRegistry.h"

#include "engine/UpdateContext.h"

#include "kigl/GLState.h"

namespace {
    static FontRegistry s_registry;
}

FontRegistry& FontRegistry::get() noexcept
{
    return s_registry;
}

FontRegistry::FontRegistry()
{
    // NOTE KI reserve 0 for null font
    registerFont({});
}

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

//text::FontAtlas* FontRegistry::modifyFont(text::font_id id)
//{
//    if (id < 1) return nullptr;
//
//    std::shared_lock lock(m_lock);
//    assert(id > 0 && id <= m_fonts.size());
//
//    return &m_fonts[id - 1];
//}

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
    text::FontAtlas&& src)
{
    std::unique_lock lock(m_lock);

    text::font_id fontId = findFont(src);
    if (!fontId) {
        auto& font = m_fonts.emplace_back(std::move(src));
        font.m_id = static_cast<text::font_id>(m_fonts.size());
        fontId = font.m_id;
    }

    return fontId;
}

text::font_id FontRegistry::findFont(
    const text::FontAtlas& src) const noexcept
{
    const auto& it = std::find_if(
        m_fonts.begin(), m_fonts.end(),
        [&src](const auto& font) {
            return font.m_fontPath == src.m_fontPath &&
                font.m_fontSize == src.m_fontSize &&
                font.m_atlasSize == src.m_atlasSize;
        });

    return it != m_fonts.end() ? it->m_id : 0;
}
