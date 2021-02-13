#include "SkyboxRenderer.h"

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

    textureID = loadCubemap(faces);

    shader->prepare();

    if (shader->prepare()) {
        return -1;
    }

    shader->unbind();

    buffers.prepare();

    glBindVertexArray(buffers.VAO);

    glBindBuffer(GL_ARRAY_BUFFER, buffers.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(ATTR_SKYBOX_POS, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

	return 0;
}

void SkyboxRenderer::assign(Shader* shader)
{
    shader->skybox.set(assets.skyboxUnitIndex);
    glActiveTexture(assets.skyboxUnitId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
}

void SkyboxRenderer::update(const RenderContext& ctx)
{
}

void SkyboxRenderer::render(const RenderContext& ctx)
{
    shader->bind();
    shader->skybox.set(assets.skyboxUnitIndex);

    // remove translation from the view matrix
    glm::mat4 viewMatrix = glm::mat4(glm::mat3(ctx.view));

    shader->viewMatrix.set(viewMatrix);
    shader->projectionMatrix.set(ctx.projection);

    glActiveTexture(assets.skyboxUnitId);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glDepthFunc(GL_LEQUAL);
    glBindVertexArray(buffers.VAO);
    glEnableVertexAttribArray(ATTR_SKYBOX_POS);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
}

// loads a cubemap texture from 6 individual texture faces
// order:
// +X (right)
// -X (left)
// +Y (top)
// -Y (bottom)
// +Z (front) 
// -Z (back)
// -------------------------------------------------------
unsigned int SkyboxRenderer::loadCubemap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    for (unsigned int i = 0; i < faces.size(); i++)
    {
        Image* image = Image::getImage(faces[i]);
        if (!image->load(false)) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
