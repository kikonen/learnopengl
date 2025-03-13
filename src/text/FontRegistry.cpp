#include "FontRegistry.h"

#include "engine/UpdateContext.h"

#include "kigl/GLState.h"

namespace
{
    static text::FontRegistry* s_engine{ nullptr };
}

namespace text
{
    void FontRegistry::init() noexcept
    {
        s_engine = new FontRegistry();
    }

    void FontRegistry::release() noexcept
    {
        auto* s = s_engine;
        s_engine = nullptr;
        delete s;
    }

    FontRegistry& FontRegistry::get() noexcept
    {
        assert(s_engine);
        return *s_engine;
    }
}

namespace text {
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
                return font == src;
            });

        return it != m_fonts.end() ? it->m_id : 0;
    }
}
