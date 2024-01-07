#include "SkyboxMaterial.h"

#include <filesystem>

#include "util/Util.h"

#include "asset/Shader.h"

#include "render/CubeMap.h"

#include "engine/PrepareContext.h"
#include "render/RenderContext.h"

namespace {
    std::array<std::string, 6> DEFAULT_FACES = {
        "right.jpg",
        "left.jpg",
        "top.jpg",
        "bottom.jpg",
        "front.jpg",
        "back.jpg",
    };
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
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        std::string basePath;
        {
            basePath = util::joinPath(
                ctx.m_assets.assetsDir,
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
    // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
    std::string filePath;
    {
        filePath = util::joinPath(
            ctx.m_assets.assetsDir,
            m_materialName);
    }

    m_hdriTexture.m_path = filePath;
    m_hdriTexture.prepareRT(ctx);
}

void SkyboxMaterial::prepareSkybox(
    const PrepareContext& ctx)
{
    if (!(ctx.m_assets.environmentMapEnabled && m_hdriTexture.valid())) return;

    m_skyboxMap.m_hdriTextureID = m_hdriTexture;
    m_skyboxMap.prepareRT(ctx, ctx.m_assets.skyboxSize);
}

void SkyboxMaterial::prepareEnvironment(
    const PrepareContext& ctx)
{
    if (!(ctx.m_assets.environmentMapEnabled && m_hdriTexture.valid())) return;

    m_environmentMap.m_hdriTextureID = m_hdriTexture;
    m_environmentMap.prepareRT(ctx, ctx.m_assets.environmentMapSize);
}

void SkyboxMaterial::prepareIrradiance(
    const PrepareContext& ctx)
{
    if (!(ctx.m_assets.environmentMapEnabled && m_environmentMap.valid())) return;

    m_irradianceMap.m_envCubeMapID = m_environmentMap;
    m_irradianceMap.prepareRT(ctx);
}

void SkyboxMaterial::preparePrefilter(
    const PrepareContext& ctx)
{
    if (!(ctx.m_assets.environmentMapEnabled && m_environmentMap.valid())) return;

    m_prefilterMap.m_envCubeMapID = m_environmentMap;
    m_prefilterMap.prepareRT(ctx);
}

void SkyboxMaterial::prepareBrdfLut(
    const PrepareContext& ctx)
{
    if (!(ctx.m_assets.environmentMapEnabled)) return;

    m_brdfLutTexture.prepareRT(ctx);
}

void SkyboxMaterial::bindTextures(const RenderContext& ctx)
{
    if (m_skyboxMap.valid()) {
        m_skyboxMap.bindTexture(ctx, UNIT_SKYBOX);
    } else {
        if (m_cubeMap.valid()) {
            m_cubeMap.bindTexture(ctx, UNIT_SKYBOX);
        }
    }

    if (m_environmentMap.valid()) {
        m_environmentMap.bindTexture(ctx, UNIT_ENVIRONMENT_MAP);
    }
    if (m_irradianceMap.valid()) {
        m_irradianceMap.bindTexture(ctx, UNIT_IRRADIANCE_MAP);
    }
    if (m_prefilterMap.valid()) {
        m_prefilterMap.bindTexture(ctx, UNIT_PREFILTER_MAP);
    }
    if (m_brdfLutTexture.valid()) {
        m_brdfLutTexture.bindTexture(ctx, UNIT_BDRF_LUT);
    }
}
