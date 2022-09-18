#include "SkyboxRenderer.h"

#include <filesystem>

#include "scene/CubeMap.h"

const int ATTR_SKYBOX_POS = 0;


const float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

SkyboxRenderer::SkyboxRenderer(
    const Assets& assets,
    const std::string& shaderName,
    const std::string& materialName)
: Renderer(assets),
    shaderName(shaderName),
    materialName(materialName)
{
}

SkyboxRenderer::~SkyboxRenderer()
{
}

void SkyboxRenderer::prepare(ShaderRegistry& shaders)
{
    Renderer::prepare(shaders);

    shader = shaders.getShader(assets, shaderName);

    shader->prepare();
    shader->bind();
    shader->skybox.set(assets.skyboxUnitIndex);
    shader->unbind();

    {
        std::string basePath;
        {
            std::filesystem::path fp;
            fp /= assets.modelsDir;
            fp /= materialName;
            basePath = fp.string();
        }

        std::vector<std::string> faces{
            basePath + "/right.jpg",
            basePath + "/left.jpg",
            basePath + "/top.jpg",
            basePath + "/bottom.jpg",
            basePath + "/front.jpg",
            basePath + "/back.jpg"
        };

        textureID = CubeMap::createFromImages(faces);
    }

    {
        buffers.prepare(false);

        glBindVertexArray(buffers.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(ATTR_SKYBOX_POS, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glEnableVertexAttribArray(ATTR_SKYBOX_POS);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
}

void SkyboxRenderer::assign(std::shared_ptr<Shader> shader)
{
    shader->skybox.set(assets.skyboxUnitIndex);
}

void SkyboxRenderer::bindTexture(const RenderContext& ctx)
{
    glBindTextures(assets.skyboxUnitIndex, 1, &textureID);
}

void SkyboxRenderer::update(const RenderContext& ctx, NodeRegistry& registry)
{
}

void SkyboxRenderer::render(const RenderContext& ctx, NodeRegistry& registry)
{
    shader->bind();
    bindTexture(ctx);

    // remove translation from the view matrix
    glm::mat4 viewMatrix = glm::mat4(glm::mat3(ctx.viewMatrix));

    shader->viewMatrix.set(viewMatrix);
    shader->projectionMatrix.set(ctx.projectionMatrix);

    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(buffers.VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);

    KI_GL_UNBIND(glBindVertexArray(0));
    KI_GL_UNBIND(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));
}
