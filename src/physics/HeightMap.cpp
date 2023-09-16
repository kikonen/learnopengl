#include "HeightMap.h"

#include "util/Log.h"
#include <fmt/format.h>

#include "asset/Image.h"

#include "model/Node.h"

namespace physics {
    void HeightMap::prepare()
    {
        const auto& image = *m_image;

        const int imageH = image.m_height;
        const int imageW = image.m_width;
        const int channels = image.m_channels;

        const bool image16b = image.m_is16Bbit;
        const int entrySize = channels * (image16b ? 2 : 1);
        const float entryScale = image16b ? 65535.f : 255.f;

        const size_t size = imageH * imageW;

        const float rangeYmin = m_verticalRange[0];
        const float rangeYmax = m_verticalRange[1];
        const float rangeY = rangeYmax - rangeYmin;

        int minH = 9999999;
        int maxH = -1;
        float minY = 99999999;
        float maxY = -1;

        m_heights = new float[size];

        const unsigned char* ptr = image.m_data;
        for (int i = 0; i < size; i++) {
            unsigned short heightValue = *((unsigned short*)ptr);
            float y = rangeYmin + (float)heightValue / entryScale * rangeY;

            if (heightValue < minH) minH = heightValue;
            if (heightValue > maxH) maxH = heightValue;
            if (y < minY) minY = y;
            if (y > maxY) maxY = y;

            m_heights[i] = y;
            ptr += entrySize;
        }

        KI_INFO_OUT(fmt::format(
            "HMAP: {} .. {} vs {} .. {}",
            minH, maxH, minY, maxY
        ));

        m_width = imageW;
        m_height = imageH;
    }

    float HeightMap::getTerrainHeight(float u, float v)
    {
        u = std::clamp(u, 0.f, 1.f);
        v = std::clamp(v, 0.f, 1.f);

        const float baseX = m_width * u;
        const float baseY = m_height * (1.f - v);

        float total = 0.0;
        float bias = 2.5;

        for (int x = -1; x < 2; x++) {
            for (int y = -1; y < 2; y++) {
                int mapX = (int)(baseX + x * bias);
                int mapY = (int)(baseY + y * bias);

                // TODO KI off by one bug
                // HACK KI -1 to workaround "out of bounds"
                mapX = std::clamp(mapX, 0, m_width - 1);
                mapY = std::clamp(mapY, 0, m_height - 1);

                const int offset = m_width * mapY + mapX;
#ifdef _DEBUG
                if (offset > m_height * m_width)
                    throw std::runtime_error{ "out-of-bounds" };
#endif
                total += m_heights[offset];
            }
        }

        return total / 9.0;
    }

    float HeightMap::getLevel(const glm::vec3& pos)
    {
        const auto& originPos = m_origin->getWorldPosition();

        auto diff = pos - originPos;
        //diff.x += m_worldTileSize / 2.f;
        //diff.z += m_worldTileSize / 2.f;

        const float u = diff.x / (float)m_worldSizeU;
        const float v = 1.f - diff.z / (float)m_worldSizeV;

        float h = getTerrainHeight(u, v);

        const auto& modelMat = m_origin->getModelMatrix();
        const auto p = glm::vec4{ 0.f, h, 0.f, 1.f };

        auto worldPos = modelMat * p;
        return worldPos.y;
    }
}
