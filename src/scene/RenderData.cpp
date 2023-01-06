#include "RenderData.h"

#include "asset/ImageTexture.h"


void RenderData::prepare()
{
    m_matrices.createEmpty(sizeof(MatricesUBO), GL_DYNAMIC_STORAGE_BIT);
    m_data.createEmpty(sizeof(DataUBO), GL_DYNAMIC_STORAGE_BIT);
    m_clipPlanes.createEmpty(sizeof(ClipPlanesUBO), GL_DYNAMIC_STORAGE_BIT);
    m_lights.createEmpty(sizeof(LightsUBO), GL_DYNAMIC_STORAGE_BIT);

    // Textures
    // OpenGL Superbible, 7th Edition, page 552
    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
    // https://www.khronos.org/opengl/wiki/Bindless_Texture
    // https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/        //glGenBuffers(1, &ssbo);
    //glGenBuffers(1, &m_ubo.textures);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ubo.textures);
    //glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TexturesUBO), &m_textures, GL_DYNAMIC_COPY);
    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    m_textures.prepare();
}

void RenderData::bind()
{
    m_matrices.bind(UBO_MATRICES);
    m_data.bind(UBO_DATA);
    m_clipPlanes.bind(UBO_CLIP_PLANES);
    m_lights.bind(UBO_LIGHTS);

    m_textures.m_buffer.bind(UBO_TEXTURES);
}

void RenderData::update()
{
    updateTextures();
}

void RenderData::updateMatrices(MatricesUBO& matrices)
{
    //MatricesUBO matricesUbo = {
    //    projectedMatrix,
    //    projectionMatrix,
    //    viewMatrix,
    //    lightProjectedMatrix,
    //    shadowMatrix,
    //};

    m_matrices.update(0, sizeof(MatricesUBO), &matrices);
}

void RenderData::updateData(DataUBO& data)
{
    m_data.update(0, sizeof(DataUBO), & data);
}

void RenderData::updateClipPlanes(ClipPlanesUBO& data)
{
    int count = 0;
    for (int i = 0; i < CLIP_PLANE_COUNT; i++) {
        if (!data.clipping[i].enabled) continue;
        count++;
    }

    data.clipCount = count;

    m_clipPlanes.update(0, sizeof(ClipPlanesUBO), & data);
}

void RenderData::updateLights(LightsUBO& data)
{
    m_lights.update(0, sizeof(LightsUBO), &data);
}

void RenderData::updateTextures()
{
    if (false) {
        //TexturesUBO texturesUbo;
        //memset(&texturesUbo.textures, 0, sizeof(texturesUbo.textures));

        //for (const auto& texture : ImageTexture::getPreparedTextures()) {
        //    texturesUbo.textures[texture->m_texIndex * 2] = texture->m_handle;
        //}

        ////glBindBuffer(GL_SHADER_STORAGE_BUFFER, scene->m_ubo.textures);
        ////GLvoid* p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
        ////memcpy(p, &scene->m_textures, sizeof(TexturesUBO));
        ////glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

        //glNamedBufferSubData(scene->m_ubo.textures, 0, sizeof(TexturesUBO), &texturesUbo);
    }
    else {
        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
        // https://www.khronos.org/opengl/wiki/Bindless_Texture

        auto [level, textures] = ImageTexture::getPreparedTextures();
        if (level != m_textureLevel && !textures.empty()) {
            m_textureLevel = level;

            TextureUBO entry;
            for (const auto* texture : textures) {
                if (texture->m_sent) continue;
                entry.handle = texture->m_handle;
                m_textures.set(texture->m_texIndex, entry);
                texture->m_sent = true;
            }
        }
    }
}
