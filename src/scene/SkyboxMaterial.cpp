#include "SkyboxMaterial.h"

#include <filesystem>

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
    const std::string& materialName)
    : CustomMaterial(materialName),
    m_faces{ DEFAULT_FACES }
{
}


void SkyboxMaterial::prepare(
    const Assets& assets,
    Registry* registry)
{
    {
        std::string basePath;
        {
            std::filesystem::path fp;
            fp /= assets.assetsDir;
            fp /= m_materialName;
            basePath = fp.string();
        }

        m_cubeMap.m_internalFormat = GL_RGB8;
        m_cubeMap.m_faces = {
            basePath + "/" + m_faces[0],
            basePath + "/" + m_faces[1],
            basePath + "/" + m_faces[2],
            basePath + "/" + m_faces[3],
            basePath + "/" + m_faces[4],
            basePath + "/" + m_faces[5],
        };

        if (m_swapFaces) {
            std::string tmp = m_cubeMap.m_faces[0];
            m_cubeMap.m_faces[0] = m_cubeMap.m_faces[1];
            m_cubeMap.m_faces[1] = tmp;
        }

    }
    m_cubeMap.create();
}

void SkyboxMaterial::bindTextures(const RenderContext& ctx)
{
    m_cubeMap.bindTexture(ctx, UNIT_SKYBOX);
}
