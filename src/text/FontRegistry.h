#pragma once

#include <vector>
#include <shared_mutex>

#include "text/FontAtlas.h"

struct UpdateContext;

namespace text {
    class FontRegistry {
    public:
        static FontRegistry& get() noexcept;

        FontRegistry();
        FontRegistry& operator=(const FontRegistry&) = delete;

        ~FontRegistry();

        void prepareRT();

        void updateRT(const UpdateContext& ctx);

        text::FontAtlas* getFont(text::font_id id) noexcept
        {
            if (id < 1) return nullptr;

            std::shared_lock lock(m_lock);
            if (id < 1 || id > m_fonts.size()) return nullptr;

            return &m_fonts[id - 1];
        }

        text::font_id registerFont(
            text::FontAtlas&& src);

    private:
        text::font_id findFont(
            const text::FontAtlas& src) const noexcept;

    private:
        mutable std::shared_mutex m_lock{};

        std::vector<text::FontAtlas> m_fonts;
    };
}
