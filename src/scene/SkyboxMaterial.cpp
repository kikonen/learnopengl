#include "SkyboxMaterial.h"

#include <filesystem>

#include "util/util.h"
#include "util/file.h"

#include "asset/Assets.h"

#include "shader/Shader.h"

#include "render/CubeMap.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"

#include "registry/VaoRegistry.h"

namespace {
    const std::array<std::string, 6> DEFAULT_FACES = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg",
    };

    void bindDefaultVao() {
        VaoRegistry::get().bindDefaultVao();
    }
}

const std::array<std::string, 6>& SkyboxMaterial::getDefaultFaces() {
    return DEFAULT_FACES;
}

SkyboxMaterial::SkyboxMaterial(
    const std::string& materialName,
    bool gammaCorrect)
    : CustomMaterial(materialName, gammaCorrect),
    m_faces{ DEFAULT_FACES }
{
}

const kigl::GLTextureHandle& SkyboxMaterial::getHdriTextureHandle() const
{
    return m_hdriTexture.m_texture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getBrdfLutTextureHandle() const
{
    return m_brdfLutTexture.m_texture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getIrradianceCubeMapTextureHandle() const
{
    return m_irradianceMap.m_cubeTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getPrefilterCubeMapTextureHandle() const
{
    return m_prefilterMap.m_cubeTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getEnvironmentCubeMapTextureHandle() const
{
    return m_environmentMap.m_cubeTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getSkyboxCubeMapTextureHandle() const
{
    return m_skyboxMap.m_cubeTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getIrradianceFlatTextureHandle() const
{
    return m_irradianceMap.m_flatTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getPrefilterFlatTextureHandle() const
{
    return m_prefilterMap.m_flatTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getEnvironmentFlatTextureHandle() const
{
    return m_environmentMap.m_flatTexture;
}

const kigl::GLTextureHandle& SkyboxMaterial::getSkyboxFlatTextureHandle() const
{
    return m_skyboxMap.m_flatTexture;
}

void SkyboxMaterial::prepareRT(
    const PrepareContext& ctx)
{
    if (m_hdri) {
        prepareHdri(ctx);
        prepareEnvironment(ctx);
        prepareSkybox(ctx);
    }
    else {
        prepareFaces(ctx);
    }

    prepareIrradiance(ctx);
    preparePrefilter(ctx);
    prepareBrdfLut(ctx);
}

void SkyboxMaterial::prepareFaces(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        std::string basePath;
        {
            basePath = util::joinPath(
                assets.assetsDir,
                m_materialName);
        }

        // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
        m_cubeMap.m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
        m_cubeMap.m_faces = {
            util::joinPath(basePath, m_faces[0]),
            util::joinPath(basePath, m_faces[1]),
            util::joinPath(basePath, m_faces[2]),
            util::joinPath(basePath, m_faces[3]),
            util::joinPath(basePath, m_faces[4]),
            util::joinPath(basePath, m_faces[5]),
        };

        if (m_swapFaces) {
            std::string tmp = m_cubeMap.m_faces[0];
            m_cubeMap.m_faces[0] = m_cubeMap.m_faces[1];
            m_cubeMap.m_faces[1] = tmp;
        }

    }
    m_cubeMap.prepareRT(ctx);
}

void SkyboxMaterial::prepareHdri(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
    std::string filePath;
    {
        filePath = util::joinPath(
            assets.assetsDir,
            m_materialName);
    }

    bindDefaultVao();

    m_hdriTexture.m_path = filePath;
    m_hdriTexture.prepareRT(ctx);
}

void SkyboxMaterial::prepareSkybox(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    if (!(assets.environmentMapEnabled && m_hdriTexture.valid())) return;

    bindDefaultVao();

    m_skyboxMap.m_hdriTextureID = m_hdriTexture;
    m_skyboxMap.prepareRT(ctx, assets.skyboxSize);
}

void SkyboxMaterial::prepareEnvironment(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    if (!(assets.environmentMapEnabled && m_hdriTexture.valid())) return;

    bindDefaultVao();

    m_environmentMap.m_hdriTextureID = m_hdriTexture;
    m_environmentMap.prepareRT(ctx, assets.environmentMapSize);
}

void SkyboxMaterial::prepareIrradiance(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    if (!(assets.environmentMapEnabled && m_environmentMap.valid())) return;

    bindDefaultVao();

    m_irradianceMap.m_envCubeMapID = m_environmentMap;
    m_irradianceMap.prepareRT(ctx);
}

void SkyboxMaterial::preparePrefilter(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    if (!(assets.environmentMapEnabled && m_environmentMap.valid())) return;

    bindDefaultVao();

    m_prefilterMap.m_envCubeMapID = m_environmentMap;
    m_prefilterMap.prepareRT(ctx);
}

void SkyboxMaterial::prepareBrdfLut(
    const PrepareContext& ctx)
{
    const auto& assets = ctx.getAssets();

    if (!(assets.environmentMapEnabled)) return;

    bindDefaultVao();

    m_brdfLutTexture.prepareRT(ctx);
}

void SkyboxMaterial::bindTextures(kigl::GLState& state)
{
    if (m_skyboxMap.valid()) {
        m_skyboxMap.bindTexture(state, UNIT_SKYBOX);
    } else {
        if (m_cubeMap.valid()) {
            m_cubeMap.bindTexture(state, UNIT_SKYBOX);
        }
    }

    if (m_environmentMap.valid()) {
        m_environmentMap.bindTexture(state, UNIT_ENVIRONMENT_MAP);
    }
    if (m_irradianceMap.valid()) {
        m_irradianceMap.bindTexture(state, UNIT_IRRADIANCE_MAP);
    }
    if (m_prefilterMap.valid()) {
        m_prefilterMap.bindTexture(state, UNIT_PREFILTER_MAP);
    }
    if (m_brdfLutTexture.valid()) {
        m_brdfLutTexture.bindTexture(state, UNIT_BDRF_LUT);
    }
}
