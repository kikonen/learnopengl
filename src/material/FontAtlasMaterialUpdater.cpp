#include "FontAtlasMaterialUpdater.h"

#include <fmt/format.h>

#include "util/debug.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "render/RenderContext.h"
#include "debug/DebugContext.h"

#include "text/FontAtlas.h"
#include "text/FontRegistry.h"

namespace {
}

FontAtlasMaterialUpdater::FontAtlasMaterialUpdater(
    ki::material_updater_id id,
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
    const render::RenderContext& ctx)
{
    const auto& dbg = debug::DebugContext::get();

    bool changed = m_fontId != dbg.m_showFontId;

    if (changed || !m_layer) {
        m_fontId = dbg.m_showFontId;

        auto* fontAtlas = text::FontRegistry::get().getFontAtlas(m_fontId).get();
        if (!fontAtlas || !fontAtlas->valid()) {
            fontAtlas = text::FontRegistry::get().getDefaultFontAtlas().get();
        }

        auto layer = fontAtlas ? fontAtlas->getTextureLayer() : 0;

        if (m_layer != layer) {
            m_layer = layer;
            setNeedUpdate(true);
        }
    }
}

GLuint FontAtlasMaterialUpdater::getTexLayer(material::TextureType type) const noexcept
{
    if (type == material::TextureType::map_font_atlas) {
        return m_layer;
    }
    return 0;
}
