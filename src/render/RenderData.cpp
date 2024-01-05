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
}

namespace render {
    RenderData::RenderData()
    {
        m_lightsUbo = std::make_unique<LightsUBO>();
    }

    void RenderData::prepare()
    {
        int bufferAlignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);

        m_matrices.prepare(bufferAlignment, false);
        m_data.prepare(bufferAlignment, false);
        m_bufferInfo.prepare(bufferAlignment, false);
        m_clipPlanes.prepare(bufferAlignment, false);
        m_lights.prepare(bufferAlignment, false);

        // NOTE KI m_textures *NOT* needede any longer
        // => 64bit index stored directly in material
        // Textures
        // OpenGL Superbible, 7th Edition, page 552
        // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
        // https://www.khronos.org/opengl/wiki/Bindless_Texture
        // https://www.geeks3d.com/20140704/tutorial-introduction-to-opengl-4-3-shader-storage-buffers-objects-ssbo-demo/        //glGenBuffers(1, &ssbo);
        //m_textures.prepare(1, false);
    }

    void RenderData::bind()
    {
        //MatricesUBO& ubo = m_matrices.data();

        //m_matrices.bind(UBO_MATRICES, false, 1);
        //m_data.bind(UBO_DATA, false, 1);
        //m_bufferInfo.bind(UBO_BUFFER_INFO, false, 1);
        //m_clipPlanes.bind(UBO_CLIP_PLANES, false, 1);
        //m_lights.bind(UBO_LIGHTS, false, 1);

        //m_textures.m_buffer.bind(UBO_TEXTURES);
    }

    void RenderData::update()
    {
        //updateImageTextures();
        //updateChannelTextures();
    }

    void RenderData::updateMatrices(MatricesUBO& data)
    {
        m_matrices.set(0, data);
        m_matrices.flush();
        m_matrices.bind(UBO_MATRICES, false, 1);
        m_matrices.next(false);
    }

    void RenderData::updateData(DataUBO& data)
    {
        m_data.set(0, data);
        m_data.flush();
        m_data.bind(UBO_DATA, false, 1);
        m_data.next(false);
    }

    void RenderData::updateBufferInfo(BufferInfoUBO& data)
    {
        m_bufferInfo.set(0, data);
        m_bufferInfo.flush();
        m_bufferInfo.bind(UBO_BUFFER_INFO, false, 1);
        m_bufferInfo.next(false);
    }

    void RenderData::updateClipPlanes(ClipPlanesUBO& data)
    {
        m_clipPlanes.set(0, data);
        m_clipPlanes.flush();
        m_clipPlanes.bind(UBO_CLIP_PLANES, false, 1);
        m_clipPlanes.next(false);
    }

    void RenderData::updateLights(Registry* registry, bool useLight)
    {
        LightsUBO& lightsUbo = *m_lightsUbo;

        if (!useLight) {
            lightsUbo.u_dirCount = 0;
            lightsUbo.u_pointCount = 0;
            lightsUbo.u_spotCount = 0;
        }

        if (useLight) {
            auto* node = registry->m_nodeRegistry->getDirLightNode();
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
            const auto& nodes = registry->m_nodeRegistry->getPointLightNodes();
            for (auto* node : nodes) {
                if (count >= MAX_LIGHT_COUNT) break;
                if (!node->m_light->m_enabled) continue;

                lightsUbo.u_pointLights[count] = node->m_light->toPointightUBO();
                count++;
            }
            lightsUbo.u_pointCount = count;

            int diff = static_cast<int>(nodes.size()) - MAX_LIGHT_COUNT;
            if (diff > 0) {
                KI_INFO_OUT(fmt::format("SKIPPED POINT_LIGHTS: {}", diff));
            }
        }

        if (useLight) {
            int count = 0;
            const auto& nodes = registry->m_nodeRegistry->getSpotLightNodes();
            for (auto* node : nodes) {
                if (count >= MAX_LIGHT_COUNT) break;
                if (!node->m_light->m_enabled) continue;

                lightsUbo.u_spotLights[count] = node->m_light->toSpotLightUBO();
                count++;
            }
            lightsUbo.u_spotCount = count;

            int diff = static_cast<int>(registry->m_nodeRegistry->getSpotLightNodes().size()) - MAX_LIGHT_COUNT;
            if (diff > 0) {
                KI_INFO_OUT(fmt::format("SKIPPED SPOT_LIGHTS: {}", diff));
            }
        }

        m_lights.set(0, lightsUbo);
        m_lights.flush();
        m_lights.bind(UBO_LIGHTS, false, 1);
        m_lights.next(false);
    }

    //void RenderData::updateImageTextures()
    //{
    //    // OpenGL Superbible, 7th Edition, page 552
    //    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
    //    // https://www.khronos.org/opengl/wiki/Bindless_Texture
    //
    //    auto [level, textures] = ImageTexture::getPreparedTextures();
    //    if (level != m_imageTextureLevel && !textures.empty()) {
    //        m_imageTextureLevel = level;
    //
    //        TextureUBO entry;
    //        for (const auto* texture : textures) {
    //            if (texture->m_sent) continue;
    //            entry.handle = texture->m_handle;
    //            m_textures.set(texture->m_texIndex, entry);
    //            texture->m_sent = true;
    //        }
    //
    //        m_textures.flush();
    //    }
    //}

    //void RenderData::updateChannelTextures()
    //{
    //    // OpenGL Superbible, 7th Edition, page 552
    //    // https://sites.google.com/site/john87connor/indirect-rendering/2-a-using-bindless-textures
    //    // https://www.khronos.org/opengl/wiki/Bindless_Texture
    //
    //    auto [level, textures] = ChannelTexture::getPreparedTextures();
    //    if (level != m_channelTextureLevel && !textures.empty()) {
    //        m_channelTextureLevel = level;
    //
    //        TextureUBO entry;
    //        for (const auto* texture : textures) {
    //            if (texture->m_sent) continue;
    //            entry.handle = texture->m_handle;
    //            m_textures.set(texture->m_texIndex, entry);
    //            texture->m_sent = true;
    //        }
    //
    //        m_textures.flush();
    //    }
    //}
}
