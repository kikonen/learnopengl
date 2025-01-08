#include "FontAtlasMaterialUpdater.h"

#include <fmt/format.h>

#include "util/debug.h"

#include "kigl/kigl.h"
#include "kigl/GLState.h"

#include "render/RenderContext.h"

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

GLuint64 FontAtlasMaterialUpdater::getTexHandle(TextureType type) const noexcept
{
    if (type == TextureType::diffuse) {
        const auto* atlas = text::FontRegistry::get().getFont(1);
        return atlas ? atlas->getTextureHandle() : 0;
    }
    return 0;
}
