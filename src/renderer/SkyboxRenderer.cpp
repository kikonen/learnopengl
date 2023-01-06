#include "SkyboxRenderer.h"

#include <filesystem>

#include "asset/ShaderBind.h"
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
    ShaderRegistry& shaders,
    MaterialRegistry& materialRegistry)
{
    if (m_prepared) return;
    m_prepared = true;

    Renderer::prepare(assets, shaders, materialRegistry);

    m_shader = shaders.getShader(assets, m_shaderName);

    m_shader->prepare(assets);

    {
        std::string basePath;
        {
            std::filesystem::path fp;
            fp /= assets.modelsDir;
            fp /= m_materialName;
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

        m_textureID = CubeMap::createFromImages(faces);
    }

    {
        m_vao.create();
        m_vbo.create();

        const int vao = m_vao;

        m_vbo.init(sizeof(skyboxVertices), (void*) & skyboxVertices, 0);

        glVertexArrayVertexBuffer(vao, VBO_VERTEX_BINDING, m_vbo, 0, sizeof(glm::vec3));

        glEnableVertexArrayAttrib(vao, ATTR_SKYBOX_POS);

        glVertexArrayAttribFormat(vao, ATTR_SKYBOX_POS, 3, GL_FLOAT, GL_FALSE, 0);

        glVertexArrayAttribBinding(vao, ATTR_SKYBOX_POS, VBO_VERTEX_BINDING);
    }
}

void SkyboxRenderer::assign(Shader* shader)
{
//    shader->skybox.set(ctx.assets.skyboxUnitIndex);
}

void SkyboxRenderer::bindTexture(const RenderContext& ctx)
{
    ctx.state.bindTexture(UNIT_SKYBOX, m_textureID, false);
}

void SkyboxRenderer::render(const RenderContext& ctx)
{
    ctx.bindDefaults();
    ShaderBind bound(m_shader, ctx.state);
    bindTexture(ctx);

    // remove translation from the view matrix
    glm::mat4 viewMatrix = glm::mat4(glm::mat3(ctx.m_matrices.view));

    m_shader->u_viewMatrix.set(viewMatrix);
    m_shader->u_projectionMatrix.set(ctx.m_matrices.projection);

    glDepthFunc(GL_LEQUAL);
    ctx.state.bindVAO(m_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);
}
