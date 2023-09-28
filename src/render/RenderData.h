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
    void updateImageTextures();
    void updateChannelTextures();

public:

private:
    std::unique_ptr<LightsUBO> m_lightsUbo;

    GLSyncQueue<MatricesUBO, false> m_matrices{ "matrices", 1, RENDER_DATA_BUFFER_COUNT, false, false };
    GLSyncQueue<DataUBO, false> m_data{ "dataUBO", 1, RENDER_DATA_BUFFER_COUNT, false, false };
    GLSyncQueue<BufferInfoUBO, false> m_bufferInfo{ "bufferInfoUBO", 1, RENDER_DATA_BUFFER_COUNT, false, false };
    GLSyncQueue<ClipPlanesUBO, false> m_clipPlanes{ "cliplanesUBO", 1, RENDER_DATA_BUFFER_COUNT, false, false };
    GLSyncQueue<LightsUBO, false> m_lights{ "lightsUBO", 1, RENDER_DATA_BUFFER_COUNT, false, false };

    GLSyncQueue<TextureUBO, true> m_textures{ "textures", 1, MAX_TEXTURE_COUNT, false, false };
    int m_imageTextureLevel = -1;
    int m_channelTextureLevel = -1;
};
