#include "FontRegistry.h"

#include <fmt/format.h>

#include "debug/DebugContext.h"
#include "engine/UpdateContext.h"

#include "ki/sid.h"

#include "kigl/GLState.h"

#include "text/FontAtlas.h"

namespace
{
    static text::FontRegistry* s_registry{ nullptr };

    util::Ref<text::FontAtlas> NULL_FONT;
}

namespace text
{
    void FontRegistry::init() noexcept
    {
        assert(!s_registry);
        s_registry = new FontRegistry();
    }

    void FontRegistry::release() noexcept
    {
        auto* s = s_registry;
        s_registry = nullptr;
        delete s;
    }

    FontRegistry& FontRegistry::get() noexcept
    {
        assert(s_registry);
        return *s_registry;
    }
}

namespace text {
    FontRegistry::FontRegistry()
    {
    }

    FontRegistry::~FontRegistry()
    {
        m_fonts.clear();
        m_fontIds.clear();
    }

    void FontRegistry::clear()
    {
        m_fonts.clear();
        m_fontIds.clear();

        // NOTE KI reserve 0 for null font
        registerFont({});

        {
            const auto font = util::Ref<text::FontAtlas>::create();
            font->m_name = "Default";
            font->m_fontPath = "fonts/LuckiestGuy.ttf";
            font->m_fontSize = 16;

            m_defaultFontId = registerFont(font);
        }

        auto& dbg = debug::DebugContext::get().edit();
        dbg.m_showFontId = m_defaultFontId;
    }

    void FontRegistry::prepareRT()
    {
    }

    void FontRegistry::updateRT(const UpdateContext& ctx)
    {
        std::shared_lock lock(m_lock);

        for (auto& [fontId, font] : m_fonts) {
            font->prepare();
            font->update();
        }
    }

    const util::Ref<text::FontAtlas>& FontRegistry::getDefaultFontAtlas() const noexcept
    {
        return getFontAtlas(m_defaultFontId);
    }

    const util::Ref<text::FontAtlas>& FontRegistry::getFontAtlas(text::font_id id) const noexcept
    {
        const auto& it = m_fonts.find(id);
        return it != m_fonts.end() ? it->second : NULL_FONT;
    }

    const util::Ref<text::FontAtlas>& FontRegistry::getPreparedFontAtlas(
        text::font_id fontId,
        bool useDefault) const noexcept
    {
        const auto& fontAtlas = getFontAtlas(fontId);
        if (!fontAtlas && useDefault) {
            return getDefaultFontAtlas();
        }
        if (!fontAtlas) return NULL_FONT;
        if (!fontAtlas->getFont()) return NULL_FONT;
        return fontAtlas;
    }

    text::font_id FontRegistry::registerFont(
        const util::Ref<text::FontAtlas>& src)
    {
        std::unique_lock lock(m_lock);

        text::font_id fontId = 0;

        if (src) {
            std::string key = fmt::format(
                "{}_{}_{}",
                src->m_name,
                src->m_fontPath,
                src->m_fontSize);

            fontId = SID_REGISTER(key).asSid();
        }

        if (src) {
            const auto& it = m_fonts.find(fontId);
            if (it == m_fonts.end()) {
                src->m_id = fontId;
                m_fonts.insert({ fontId, src });
            }
            m_fontIds.push_back(fontId);
        }

        return fontId;
    }
}
