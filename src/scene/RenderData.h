#pragma once

#include "asset/UBO.h"

#include "kigl/GLSyncQueue.h"

constexpr int RENDER_DATA_QUEUE_SIZE = 16;

class RenderData {
public:
    RenderData() = default;

    void prepare();
    void bind();
    void update();

    void updateMatrices(MatricesUBO& data);
    void updateData(DataUBO& data);
    void updateClipPlanes(ClipPlanesUBO& data);
    void updateLights(LightsUBO& data);

private:
    void updateTextures();

public:

private:
    GLSyncQueue<MatricesUBO> m_matrices{ "matrices", 1, RENDER_DATA_QUEUE_SIZE, true };
    GLSyncQueue<DataUBO> m_data{ "data", 1, RENDER_DATA_QUEUE_SIZE, true };
    GLSyncQueue<ClipPlanesUBO> m_clipPlanes{ "clipPlanes", 1, RENDER_DATA_QUEUE_SIZE, true };
    GLSyncQueue<LightsUBO> m_lights{ "lights", 1, RENDER_DATA_QUEUE_SIZE, true };

    GLSyncQueue<TextureUBO> m_textures{ "textures", 1, TEXTURE_COUNT, true };
    int m_textureLevel = -1;
};
