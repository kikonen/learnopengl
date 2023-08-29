#pragma once

#include "asset/UBO.h"
#include "asset/TextureUBO.h"

#include "kigl/GLSyncQueue.h"

constexpr int RENDER_DATA_QUEUE_SIZE = 16;

class Registry;

struct MatricesUBO;
struct DataUBO;
struct BufferInfoUBO;
struct ClipPlanesUBO;
struct LightsUBO;
struct TesturUBO;

class RenderData {
public:
    RenderData() = default;

    void prepare();
    void bind();
    void update();

    void updateMatrices(MatricesUBO& data);
    void updateData(DataUBO& data);
    void updateBufferInfo(BufferInfoUBO& data);
    void updateClipPlanes(ClipPlanesUBO& data);
    void updateLights(Registry* registry, bool useLight);

private:
    void updateTextures();

public:

private:
    GLBuffer m_matrices{ "matricesUBO" };
    GLBuffer m_data{ "dataUBO" };
    GLBuffer m_bufferInfo{ "bufferInfoUBO" };
    GLBuffer m_clipPlanes{ "clipPlanesUBO" };
    GLBuffer m_lights{ "lightsUBO" };

    GLSyncQueue<TextureUBO, true> m_textures{ "textures", 1, MAX_TEXTURE_COUNT, false };
    int m_textureLevel = -1;
};
