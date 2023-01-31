#include "SkyboxRenderer.h"

#include <filesystem>

#include "scene/CubeMap.h"

#include "scene/RenderContext.h"


SkyboxRenderer::SkyboxRenderer(
    const std::string& shaderName,
    const std::string& materialName)
    : m_shaderName(shaderName),
    m_materialName(materialName)
{
}

SkyboxRenderer::~SkyboxRenderer()
{
}

void SkyboxRenderer::prepare(
    const Assets& assets,
    Registry* registry)
{
    {
        std::string basePath;
        {
            std::filesystem::path fp;
            fp /= assets.modelsDir;
            fp /= m_materialName;
            basePath = fp.string();
        }

        m_cubeMap.m_internalFormat = GL_RGB8;
        m_cubeMap.m_faces = {
            basePath + "/right.jpg",
            basePath + "/left.jpg",
            basePath + "/top.jpg",
            basePath + "/bottom.jpg",
            basePath + "/front.jpg",
            basePath + "/back.jpg"
        };

    }
    m_cubeMap.create();
}

void SkyboxRenderer::bindTexture(const RenderContext& ctx)
{
    m_cubeMap.bindTexture(ctx, UNIT_SKYBOX);
}
