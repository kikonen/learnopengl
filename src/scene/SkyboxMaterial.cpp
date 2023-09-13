#include "SkyboxMaterial.h"

#include <filesystem>

#include "util/Util.h"

#include "asset/Shader.h"

#include "render/CubeMap.h"

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

void SkyboxMaterial::prepare(
    const Assets& assets,
    Registry* registry)
{
    if (m_hdri) {
        prepareHdri(assets, registry);
    }
    else {
        prepareFaces(assets, registry);
    }
}

void SkyboxMaterial::prepareFaces(
    const Assets& assets,
    Registry* registry)
{
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
    m_cubeMap.prepare(assets, registry);
}

void SkyboxMaterial::prepareHdri(
    const Assets& assets,
    Registry* registry)
{
    {
        // NOTE KI MUST normalize path to avoid mismatches due to \ vs /
        std::string filePath;
        {
            filePath = util::joinPath(
                assets.assetsDir,
                m_materialName);
        }

        m_cubeMap.m_faces = {
            filePath,
        };

        // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
        m_cubeMap.m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
        m_cubeMap.m_hdri = true;

        m_cubeMap.prepare(assets, registry);
    }

    if (assets.irradianceMapEnabled && m_cubeMap.valid()) {
        // NOTE KI https://learnopengl.com/Advanced-Lighting/Gamma-Correction
        m_irradianceMap.m_internalFormat = m_gammaCorrect ? GL_SRGB8 : GL_RGB8;
        m_irradianceMap.m_irradiance = true;
        m_irradianceMap.m_hdriCubeMapRef = &m_cubeMap;

        m_irradianceMap.prepare(assets, registry);
    }
}

void SkyboxMaterial::bindTextures(const RenderContext& ctx)
{
    m_cubeMap.bindTexture(ctx, UNIT_SKYBOX);
    m_irradianceMap.bindTexture(ctx, UNIT_IRRADIANCE_MAP);
}
