#include "FontAtlasMaterialUpdater.h"

#include <fmt/format.h>

#include "util/debug.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "render/RenderContext.h"
#include "render/DebugContext.h"

#include "text/FontAtlas.h"
#include "text/FontRegistry.h"

namespace {
}

FontAtlasMaterialUpdater::FontAtlasMaterialUpdater(
    ki::StringID id,
    const std::string& name)
    : MaterialUpdater{ id, name }
{
}

FontAtlasMaterialUpdater::~FontAtlasMaterialUpdater()
{
}

void FontAtlasMaterialUpdater::prepareRT(
    const PrepareContext& ctx)
{
    if (m_prepared) return;
    m_prepared = true;

}

void FontAtlasMaterialUpdater::render(
    const RenderContext& ctx)
{
    const auto& dbg = render::DebugContext::get();

    bool changed = m_fontId != dbg.m_showFontId;

    if (changed || !m_handle) {
        m_fontId = dbg.m_showFontId;

        const auto* atlas = text::FontRegistry::get().getFont(m_fontId);
        auto handle = atlas ? atlas->getTextureHandle() : 0;

        if (m_handle != handle) {
            m_handle = handle;
            setNeedUpdate(true);
        }
    }
}

GLuint64 FontAtlasMaterialUpdater::getTexHandle(TextureType type) const noexcept
{
    if (type == TextureType::map_custom_1) {
        return m_handle;
    }
    return 0;
}
