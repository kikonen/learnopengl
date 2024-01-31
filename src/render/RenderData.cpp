#include "RenderData.h"

#include "asset/ImageTexture.h"
#include "asset/ChannelTexture.h"

#include "asset/MatricesUBO.h"
#include "asset/DataUBO.h"
#include "asset/BufferInfoUBO.h"
#include "asset/ClipPlaneUBO.h"
#include "asset/LightUBO.h"
#include "asset/TextureUBO.h"

#include "asset/UBO.h"

#include "kigl/GLSyncQueue_impl.h"

#include "model/Node.h"
#include "component/Light.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

namespace {
    constexpr int RENDER_DATA_BUFFER_COUNT = 32;
}

namespace render {
    RenderData::RenderData()
    {
        m_lightsUbo = std::make_unique<LightsUBO>();
    }

    void RenderData::prepare(
        bool useMapped,
        bool useInvalidate,
        bool useFence,
        bool useSingleFence,
        bool useDebugFence,
        bool debug)
    {
        int bufferAlignment;
        glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &bufferAlignment);

        m_matrices = std::make_unique<kigl::GLSyncQueue<MatricesUBO>>(
            "matrices_ubo",
            1,
            RENDER_DATA_BUFFER_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useSingleFence,
            useDebugFence);

        m_data = std::make_unique<kigl::GLSyncQueue<DataUBO>>(
            "data_ubo",
            1,
            RENDER_DATA_BUFFER_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useSingleFence,
            useDebugFence);

        m_bufferInfo = std::make_unique<kigl::GLSyncQueue<BufferInfoUBO>>(
            "buffer_info_ubo",
            1,
            RENDER_DATA_BUFFER_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useSingleFence,
            useDebugFence);

        m_clipPlanes = std::make_unique<kigl::GLSyncQueue<ClipPlanesUBO>>(
            "cliplanes_ubo",
            1,
            RENDER_DATA_BUFFER_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useSingleFence,
            useDebugFence);

        m_lights = std::make_unique<kigl::GLSyncQueue<LightsUBO>>(
            "lights_ubo",
            1,
            RENDER_DATA_BUFFER_COUNT,
            useMapped,
            useInvalidate,
            useFence,
            useSingleFence,
            useDebugFence);

        m_matrices->prepare(bufferAlignment, debug);
        m_data->prepare(bufferAlignment, debug);
        m_bufferInfo->prepare(bufferAlignment, debug);
        m_clipPlanes->prepare(bufferAlignment, debug);
        m_lights->prepare(bufferAlignment, debug);

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

    void RenderData::updateMatrices(const MatricesUBO& data)
    {
        m_matrices->set(0, data);
        m_matrices->flush();
        m_matrices->bind(UBO_MATRICES, false, 1);
        m_matrices->next();
    }

    void RenderData::updateData(const DataUBO& data)
    {
        m_data->set(0, data);
        m_data->flush();
        m_data->bind(UBO_DATA, false, 1);
        m_data->next();
    }

    void RenderData::updateBufferInfo(const BufferInfoUBO& data)
    {
        m_bufferInfo->set(0, data);
        m_bufferInfo->flush();
        m_bufferInfo->bind(UBO_BUFFER_INFO, false, 1);
        m_bufferInfo->next();
    }

    void RenderData::updateClipPlanes(const ClipPlanesUBO& data)
    {
        m_clipPlanes->set(0, data);
        m_clipPlanes->flush();
        m_clipPlanes->bind(UBO_CLIP_PLANES, false, 1);
        m_clipPlanes->next();
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
            auto& handle = NodeRegistry::get().getDirLightNode();
            auto* node = handle.toNode();

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
            auto& handles = NodeRegistry::get().getPointLightNodes();
            for (auto& handle : handles) {
                auto* node = handle.toNode();
                if (!node) continue;

                if (count >= MAX_LIGHT_COUNT) break;
                if (!node->m_light->m_enabled) continue;

                lightsUbo.u_pointLights[count] = node->m_light->toPointightUBO();
                count++;
            }
            lightsUbo.u_pointCount = count;

            const int diff = static_cast<int>(handles.size()) - MAX_LIGHT_COUNT;
            if (diff > 0) {
                KI_INFO_OUT(fmt::format("SKIPPED POINT_LIGHTS: {}", diff));
            }
        }

        if (useLight) {
            int count = 0;
            const auto& handles = NodeRegistry::get().getSpotLightNodes();
            for (auto& handle : handles) {
                auto* node = handle.toNode();
                if (!node) continue;

                if (count >= MAX_LIGHT_COUNT) break;
                if (!node->m_light->m_enabled) continue;

                lightsUbo.u_spotLights[count] = node->m_light->toSpotLightUBO();
                count++;
            }
            lightsUbo.u_spotCount = count;

            const int diff = static_cast<int>(handles.size()) - MAX_LIGHT_COUNT;
            if (diff > 0) {
                KI_INFO_OUT(fmt::format("SKIPPED SPOT_LIGHTS: {}", diff));
            }
        }

        m_lights->set(0, lightsUbo);
        m_lights->flush();
        m_lights->bind(UBO_LIGHTS, false, 1);
        m_lights->next();
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
