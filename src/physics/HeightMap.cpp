#include "HeightMap.h"

namespace physics {
    void HeightMap::prepare()
    {
        const auto& image = *m_image;

        const int imageH = image.m_height;
        const int imageW = image.m_width;
        const int channels = image.m_channels;

        const int count = imageH * imageW;

        m_heights = new float[count];

        const unsigned char* ptr = image.m_data;
        for (int i = 0; i < count; i++) {
            unsigned char h = *ptr;
            m_heights[i] = h / 255.f;
            ptr += channels;
        }
    }

    float HeightMap::getHeight(float z, float x)
    {
        int worldZI = 0;
        int worldXI = 0;
        const auto gridSize = 100;
        const auto worldTilesZ = 8;
        const auto worldTilesX = 8;

        const auto& image = *m_image;

        const int imageH = image.m_height;
        const int imageW = image.m_width;
        const int channels = image.m_channels;

        const unsigned char* data = image.m_data;
        const int dataSize = imageH * imageW * channels;

        const size_t vertexCount = gridSize + 1;

        // grid cell size
        const float tileH = (float)imageH / (float)worldTilesZ;
        const float tileW = (float)imageW / (float)worldTilesX;

        // grid texel cell size
        const float texTileH = (float)1.f / (float)worldTilesZ;
        const float texTileW = (float)1.f / (float)worldTilesX;

        // ratio per grid cell
        const float ratioH = tileH / (float)vertexCount;
        const float ratioW = tileW / (float)vertexCount;

        // ratio per grid texel cell
        const float texRatioH = texTileH / (float)gridSize;
        const float texRatioW = texTileW / (float)gridSize;

        const int baseZI = worldZI * gridSize;
        const int baseXI = worldXI * gridSize;

        for (int zi = 0; zi < vertexCount; zi++) {
            // vz = [-1, 1] => local to mesh
            float z = zi / (float)gridSize;
            z = z * 2.f - 1.f;
            z = std::clamp(z, -1.f, 1.f);

            // tz = [0, 1] => global to world
            float v = 1.f - (baseZI + zi) * texRatioH;

            for (int xi = 0; xi < vertexCount; xi++) {
                // gx = [-1, 1] => local to mesh
                float x = xi / (float)gridSize;
                x = x * 2.f - 1.f;
                x = std::clamp(x, -1.f, 1.f);

                // tx = [0, 1] => global to world
                float u = (baseXI + xi) * texRatioW;
                //tx = std::clamp(tx, 0.f, 1.f);

                //float gy = perlin.perlin(gx * tileSize, 0, gz * tileSize);
                int pz = (baseZI + zi) * ratioH;
                int px = (baseXI + xi) * ratioW;
                int offsetZ = imageW * pz;
                int offsetX = px;
                int offset = offsetZ * channels + offsetX * channels;

                auto* ptr = data;
                ptr += offset;

                unsigned char heightValue = *ptr;
                float height = (heightValue / 255.f) * m_heightScale;
                float vy = height;
            }
        }

        // TODO KI interpolate value
        return 0.f;
    }

    float HeightMap::getLevel(const glm::vec3& pos)
    {
        return 0;
    }
}
