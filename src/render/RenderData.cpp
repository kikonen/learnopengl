#include "RenderData.h"

#include "asset/ImageTexture.h"
#include "asset/ChannelTexture.h"

#include "asset/MatricesUBO.h"
#include "asset/DataUBO.h"
#include "asset/BufferInfoUBO.h"
#include "asset/ClipPlaneUBO.h"
#include "asset/LightUBO.h"
#include "asset/TextureUBO.h"

#include "model/Node.h"
#include "component/Light.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    constexpr size_t UNIFORM_BUFFER_OFFSET_ALIGNMENT = 256;
}

void RenderData::prepare()
{
    m_matrices.createEmpty(sizeof(MatricesUBO), GL_DYNAMIC_STORAGE_BIT);
    m_data.createEmpty(sizeof(DataUBO), GL_DYNAMIC_STORAGE_BIT);
    m_bufferInfo.createEmpty(sizeof(BufferInfoUBO), GL_DYNAMIC_STORAGE_BIT);
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
    m_textures.prepare(1, false);
}

void RenderData::bind()
{
    m_matrices.bind(UBO_MATRICES);
    m_data.bind(UBO_DATA);
    m_bufferInfo.bind(UBO_BUFFER_INFO);
    m_clipPlanes.bind(UBO_CLIP_PLANES);
    m_lights.bind(UBO_LIGHTS);

    m_textures.m_buffer.bind(UBO_TEXTURES);
}

void RenderData::update()
{
    updateImageTextures();
    updateChannelTextures();
}

void RenderData::updateMatrices(MatricesUBO& data)
{
    //m_matrices.update(0, sizeof(MatricesUBO), &data);
    m_matrices.update(0, sizeof(MatricesUBO), &data);
}

void RenderData::updateData(DataUBO& data)
{
    m_data.update(0, sizeof(DataUBO), &data);
}

void RenderData::updateBufferInfo(BufferInfoUBO& data)
{
    m_bufferInfo.update(0, sizeof(BufferInfoUBO), & data);
}

void RenderData::updateClipPlanes(ClipPlanesUBO& data)
{
    m_clipPlanes.update(0, sizeof(ClipPlanesUBO), &data);
}

void RenderData::updateLights(Registry* registry, bool useLight)
{
    LightsUBO lightsUbo{};
    if (!useLight) {
        lightsUbo.u_dirCount = 0;
        lightsUbo.u_pointCount = 0;
        lightsUbo.u_spotCount = 0;
    }

    if (useLight) {
        auto* node = registry->m_nodeRegistry->m_dirLight;
        if (node && node->m_light->m_enabled) {
            lightsUbo.u_dir[0] = node->m_light->toDirLightUBO();
            lightsUbo.u_dirCount = 1;
        }
        else {
            lightsUbo.u_dirCount = 0;
        }
    }

    if (useLight) {
        int count = 0;
        for (auto* node : registry->m_nodeRegistry->m_pointLights) {
            if (count >= MAX_LIGHT_COUNT) break;
            if (!node->m_light->m_enabled) continue;

            lightsUbo.u_pointLights[count] = node->m_light->toPointightUBO();
            count++;
        }
        lightsUbo.u_pointCount = count;

        int diff = registry->m_nodeRegistry->m_pointLights.size() - MAX_LIGHT_COUNT;
        if (diff > 0) {
            KI_INFO_OUT(fmt::format("SKIPPED POINT_LIGHTS: {}", diff));
        }
    }

    if (useLight) {
        int count = 0;
        for (auto* node : registry->m_nodeRegistry->m_spotLights) {
            if (count >= MAX_LIGHT_COUNT) break;
            if (!node->m_light->m_enabled) continue;

            lightsUbo.u_spotLights[count] = node->m_light->toSpotLightUBO();
            count++;
        }
        lightsUbo.u_spotCount = count;

        int diff = registry->m_nodeRegistry->m_spotLights.size() - MAX_LIGHT_COUNT;
        if (diff > 0) {
            KI_INFO_OUT(fmt::format("SKIPPED SPOT_LIGHTS: {}", diff));
        }
    }

    m_lights.update(0, sizeof(LightsUBO), &lightsUbo);
}

void RenderData::updateImageTextures()
{
    // OpenGL Superbible, 7th Edition, page 552
    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
    // https://www.khronos.org/opengl/wiki/Bindless_Texture

    auto [level, textures] = ImageTexture::getPreparedTextures();
    if (level != m_imageTextureLevel && !textures.empty()) {
        m_imageTextureLevel = level;

        TextureUBO entry;
        for (const auto* texture : textures) {
            if (texture->m_sent) continue;
            entry.handle = texture->m_handle;
            m_textures.set(texture->m_texIndex, entry);
            texture->m_sent = true;
        }

        m_textures.flush();
    }
}

void RenderData::updateChannelTextures()
{
    // OpenGL Superbible, 7th Edition, page 552
    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
    // https://www.khronos.org/opengl/wiki/Bindless_Texture

    auto [level, textures] = ChannelTexture::getPreparedTextures();
    if (level != m_channelTextureLevel && !textures.empty()) {
        m_channelTextureLevel = level;

        TextureUBO entry;
        for (const auto* texture : textures) {
            if (texture->m_sent) continue;
            entry.handle = texture->m_handle;
            m_textures.set(texture->m_texIndex, entry);
            texture->m_sent = true;
        }

        m_textures.flush();
    }
}
