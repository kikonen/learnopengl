#include "FontRegistry.h"

#include <fmt/format.h>

#include "engine/UpdateContext.h"

#include "ki/sid.h"

#include "kigl/GLState.h"

namespace
{
    static text::FontRegistry* s_registry{ nullptr };
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
    }

    void FontRegistry::clear()
    {
        m_fonts.clear();

        // NOTE KI reserve 0 for null font
        registerFont({});

        {
            text::FontAtlas font;
            font.m_name = "Default";
            font.m_fontPath = "fonts/LuckiestGuy.ttf";
            font.m_fontSize = 16;

            m_defaultFontId = registerFont(std::move(font));
        }
    }

    void FontRegistry::prepareRT()
    {
    }

    void FontRegistry::updateRT(const UpdateContext& ctx)
    {
        std::shared_lock lock(m_lock);

        for (auto& [fontId, font] : m_fonts) {
            font.prepare();
            font.update();
        }
    }

    text::font_id FontRegistry::registerFont(
        text::FontAtlas&& src)
    {
        std::unique_lock lock(m_lock);

        text::font_id fontId;
        {
            std::string key = fmt::format(
                "{}_{}_{}",
                src.m_name,
                src.m_fontPath,
                src.m_fontSize);

            fontId = SID(key);
        }

        const auto& it = m_fonts.find(fontId);
        if (it == m_fonts.end())
        {
            src.m_id = fontId;
            m_fonts.insert({ fontId, std::move(src) });
        }

        return fontId;
    }
}
