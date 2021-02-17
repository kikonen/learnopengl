#include "SkyboxRenderer.h"

#include "CubeMap.h"

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

SkyboxRenderer::SkyboxRenderer(const Assets& assets, const std::string& name)
	: Renderer(assets),
    name(name)
{
    shader = Shader::getShader(assets, name);
}

SkyboxRenderer::~SkyboxRenderer()
{
}

int SkyboxRenderer::prepare()
{
    shader->prepare();
    shader->bind();
    shader->skybox.set(assets.skyboxUnitIndex);
    shader->unbind();

    {
        const std::string& baseDir = assets.modelsDir;
        std::string texturePath = baseDir + name;

        std::vector<std::string> faces{
            baseDir + "/" + name + "/right.jpg",
            baseDir + "/" + name + "/left.jpg",
            baseDir + "/" + name + "/top.jpg",
            baseDir + "/" + name + "/bottom.jpg",
            baseDir + "/" + name + "/front.jpg",
            baseDir + "/" + name + "/back.jpg"
        };

        textureID = CubeMap::createFromImages(faces);
    }

    {
        buffers.prepare();

        glBindVertexArray(buffers.VAO);

        glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

        glVertexAttribPointer(ATTR_SKYBOX_POS, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

	return 0;
}

void SkyboxRenderer::assign(Shader* shader)
{
    shader->skybox.set(assets.skyboxUnitIndex);
}

void SkyboxRenderer::bindTexture(const RenderContext& ctx)
{
    glActiveTexture(assets.skyboxUnitId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void SkyboxRenderer::update(const RenderContext& ctx)
{
}

void SkyboxRenderer::render(const RenderContext& ctx)
{
    shader->bind();
    bindTexture(ctx);

    // remove translation from the view matrix
    glm::mat4 viewMatrix = glm::mat4(glm::mat3(ctx.view));

    shader->viewMatrix.set(viewMatrix);
    shader->projectionMatrix.set(ctx.projection);

    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(buffers.VAO);
    glEnableVertexAttribArray(ATTR_SKYBOX_POS);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
