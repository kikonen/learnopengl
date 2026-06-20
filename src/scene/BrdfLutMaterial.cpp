#include "BrdfLutMaterial.h"

#include <filesystem>

#include "util/util.h"
#include "util/file.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "render/CubeMap.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"

#include "registry/VaoRegistry.h"

namespace
{
    void bindDefaultVao()
    {
        VaoRegistry::get().bindDefaultVao();
    }
}

BrdfLutMaterial::BrdfLutMaterial()
    : CustomMaterial("brdf_lut", false)
{
}

const kigl::GLTextureHandle& BrdfLutMaterial::getBrdfLutTextureHandle() const
{
    return m_brdfLutTexture.m_texture;
}

void BrdfLutMaterial::prepareRT(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    if (!(assets.environmentMapEnabled)) return;

    bindDefaultVao();

    m_brdfLutTexture.prepareRT(ctx);
}

void BrdfLutMaterial::bindTextures(kigl::GLState& state)
{
    if (m_brdfLutTexture.valid()) {
        m_brdfLutTexture.bindTexture(state, UNIT_BDRF_LUT);
    }
}
