#pragma once

#include "asset/UBO.h"
#include "asset/TextureUBO.h"

#include "kigl/GLSyncQueue.h"

constexpr int RENDER_DATA_BUFFER_COUNT = 16;

class Registry;

struct MatricesUBO;
struct DataUBO;
struct BufferInfoUBO;
struct ClipPlanesUBO;
struct LightsUBO;
struct TesturUBO;

namespace render {
    class RenderData {
    public:
        RenderData();

        void prepare();
        void bind();
        void update();

        void updateMatrices(MatricesUBO& data);
        void updateData(DataUBO& data);
        void updateBufferInfo(BufferInfoUBO& data);
        void updateClipPlanes(ClipPlanesUBO& data);
        void updateLights(Registry* registry, bool useLight);

    private:
        //void updateImageTextures();
        //void updateChannelTextures();

    public:

    private:
        std::unique_ptr<LightsUBO> m_lightsUbo;

        kigl::GLSyncQueue<MatricesUBO, false> m_matrices{ "matrices_ubo", 1, RENDER_DATA_BUFFER_COUNT, false, false };
        kigl::GLSyncQueue<DataUBO, false> m_data{ "data_ubo", 1, RENDER_DATA_BUFFER_COUNT, false, false };
        kigl::GLSyncQueue<BufferInfoUBO, false> m_bufferInfo{ "buffer_info_ubo", 1, RENDER_DATA_BUFFER_COUNT, false, false };
        kigl::GLSyncQueue<ClipPlanesUBO, false> m_clipPlanes{ "cliplanes_ubo", 1, RENDER_DATA_BUFFER_COUNT, false, false };
        kigl::GLSyncQueue<LightsUBO, false> m_lights{ "lights_ubo", 1, RENDER_DATA_BUFFER_COUNT, false, false };

        //kigl::GLSyncQueue<TextureUBO, true> m_textures{ "textures", 1, MAX_TEXTURE_COUNT, false, false };
        //int m_imageTextureLevel = -1;
        //int m_channelTextureLevel = -1;
    };
}
