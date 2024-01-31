#pragma once

#include <vector>
#include <shared_mutex>

#include "text/FontAtlas.h"

struct UpdateContext;

class FontRegistry {
public:
    FontRegistry();

    ~FontRegistry();

    void prepareRT();

    void updateRT(const UpdateContext& ctx);

    text::FontAtlas* getFont(text::font_id id) noexcept
    {
        if (id < 1) return nullptr;

        std::shared_lock lock(m_lock);
        assert(id > 0 && id <= m_fonts.size());

        return &m_fonts[id - 1];
    }

    text::FontAtlas* modifyFont(text::font_id id);

    bool bindFont(
        text::font_id id);

    bool unbindFont(
        text::font_id id);

    text::font_id registerFont(
        const std::string& name);

private:
    mutable std::shared_mutex m_lock{};

    std::vector<text::FontAtlas> m_fonts;
};
