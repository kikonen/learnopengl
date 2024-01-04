#pragma once

#include <vector>
#include <mutex>

#include "asset/Assets.h"

#include "text/FontAtlas.h"

namespace kigl {
    class GLState;
}

class UpdateContext;

class FontRegistry {
public:
    FontRegistry(
        const Assets& assets);

    ~FontRegistry();

    void prepareRT();

    void updateRT(const UpdateContext& ctx);

    text::FontAtlas* getFont(text::font_id id) noexcept
    {
        if (id < 1) return nullptr;

        std::lock_guard<std::mutex> lock(m_lock);
        assert(id > 0 && id <= m_fonts.size());

        return &m_fonts[id - 1];
    }

    text::FontAtlas* modifyFont(text::font_id id);

    bool bindFont(
        kigl::GLState& state,
        text::font_id id);

    bool unbindFont(
        kigl::GLState& state,
        text::font_id id);

    text::font_id registerFont(
        const std::string& name);

private:
    const Assets& m_assets;

    mutable std::mutex m_lock{};

    std::vector<text::FontAtlas> m_fonts;
};
