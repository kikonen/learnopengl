#pragma once

#include <vector>
#include <unordered_map>
#include <shared_mutex>

#include "util/Ref.h"

#include "text/size.h"

struct UpdateContext;

namespace text {
    class FontAtlas;

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

        const util::Ref<text::FontAtlas>& getDefaultFontAtlas() const noexcept;

        const util::Ref<text::FontAtlas>& getFontAtlas(text::font_id id) const noexcept;

        const util::Ref<text::FontAtlas>& getPreparedFontAtlas(
            text::font_id id,
            bool useDefault) const noexcept;

        text::font_id registerFont(
            const util::Ref<text::FontAtlas>& src);

        const std::vector<text::font_id>& getFontIds() const noexcept
        {
            return m_fontIds;
        }

    private:
        mutable std::shared_mutex m_lock{};

        std::unordered_map<text::font_id, util::Ref<text::FontAtlas>> m_fonts;
        std::vector<text::font_id> m_fontIds;

        text::font_id m_defaultFontId{ 0 };
    };
}
