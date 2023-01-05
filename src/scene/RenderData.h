#pragma once

#include "asset/UBO.h"

#include "kigl/GLSyncQueue.h"

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
    UBO m_ubo;

    GLSyncQueue<TextureUBO> m_textures{ 1, TEXTURE_COUNT };
    int m_textureLevel = -1;
};
