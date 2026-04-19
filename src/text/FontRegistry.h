#pragma once

#include <vector>
#include <unordered_map>
#include <shared_mutex>

#include "text/FontAtlas.h"

struct UpdateContext;

namespace text {
    class FontRegistry {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static FontRegistry& get() noexcept;

        FontRegistry();
        FontRegistry& operator=(const FontRegistry&) = delete;

        ~FontRegistry();

        void clear();
        void prepareRT();

        void updateRT(const UpdateContext& ctx);

        const text::FontAtlas* getDefaultFontAtlas() const noexcept
        {
            return getFontAtlas(m_defaultFontId);
        }

        const text::FontAtlas* getFontAtlas(text::font_id id) const noexcept
        {
            const auto& it = m_fonts.find(id);
            return it != m_fonts.end() ? &it->second : nullptr;
        }

        text::font_id registerFont(
            text::FontAtlas&& src);

        const std::vector<text::font_id>& getFontIds() const noexcept
        {
            return m_fontIds;
        }

    private:
        mutable std::shared_mutex m_lock{};

        std::unordered_map<text::font_id, text::FontAtlas> m_fonts;
        std::vector<text::font_id> m_fontIds;

        text::font_id m_defaultFontId{ 0 };
    };
}
