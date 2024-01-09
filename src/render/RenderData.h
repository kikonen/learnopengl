#pragma once

#include "kigl/GLSyncQueue.h"

class Registry;

struct MatricesUBO;
struct DataUBO;
struct BufferInfoUBO;
struct ClipPlanesUBO;
struct LightsUBO;

namespace render {
    class RenderData {
    public:
        RenderData();

        void prepare(bool useFence, bool debug);
        void bind();
        void update();

        void updateMatrices(const MatricesUBO& data);
        void updateData(const DataUBO& data);
        void updateBufferInfo(const BufferInfoUBO& data);
        void updateClipPlanes(const ClipPlanesUBO& data);
        void updateLights(Registry* registry, bool useLight);

    private:
        //void updateImageTextures();
        //void updateChannelTextures();

    public:

    private:
        std::unique_ptr<LightsUBO> m_lightsUbo;

        std::unique_ptr<kigl::GLSyncQueue<MatricesUBO, false>> m_matrices;
        std::unique_ptr<kigl::GLSyncQueue<DataUBO, false>> m_data;
        std::unique_ptr<kigl::GLSyncQueue<BufferInfoUBO, false>> m_bufferInfo;
        std::unique_ptr<kigl::GLSyncQueue<ClipPlanesUBO, false>> m_clipPlanes;
        std::unique_ptr<kigl::GLSyncQueue<LightsUBO, false>> m_lights;

        //kigl::GLSyncQueue<TextureUBO, true> m_textures{ "textures", 1, MAX_TEXTURE_COUNT, false, false };
        //int m_imageTextureLevel = -1;
        //int m_channelTextureLevel = -1;
    };
}
