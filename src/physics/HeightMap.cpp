#include "HeightMap.h"

#include <memory>
#include <algorithm>

#include "util/Log.h"
#include <fmt/format.h>

#include "asset/Image.h"

#include "model/Node.h"

namespace physics {
    HeightMap::HeightMap()
    {}

    HeightMap::HeightMap(HeightMap&& o)
        : m_id{ o.m_id },
        //m_image{ std::move(o.m_image) },
        m_origin{ o.m_origin },
        m_worldTileSize{ o.m_worldTileSize },
        m_worldSizeU{ o.m_worldSizeU },
        m_worldSizeV{ o.m_worldSizeV },
        m_verticalRange{ o.m_verticalRange },
        m_horizontalScale{ o.m_horizontalScale },
        m_height{ o.m_height },
        m_width{ o.m_width },
        m_heights{ o.m_heights }
    {
        // NOTE KI o is moved now
        o.m_heights = nullptr;
    }

    HeightMap::~HeightMap()
    {
        delete[] m_heights;
    }

    void HeightMap::prepare(Image* _image)
    {
        const auto& image = *_image;

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
        float minY = 99999999.f;
        float maxY = -1.f;

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

    float HeightMap::getTerrainHeight(float u, float v) const noexcept
    {
        // NOTE KI use bilinear interpolation
        // use "clamp to edge"

        const float mapX = ((float)(m_width)) * u;
        const float mapY = ((float)(m_height)) * (1.f - v);

        // floor
        int x = mapX;
        int y = mapY;

        const float fractX = mapX - x;
        const float fractY = mapY - y;

        x = std::clamp(x, 0, m_width - 1);
        y = std::clamp(y, 0, m_height - 1);

        int nextX = std::clamp(x + 1, 0, m_width - 1);
        int nextY = std::clamp(y + 1, 0, m_height - 1);

        const float h00 = m_heights[m_width * y       + x];
        const float h10 = m_heights[m_width * nextY   + x];
        const float h01 = m_heights[m_width * y       + nextX];
        const float h11 = m_heights[m_width * nextY   + nextX];

        const float bottomH = (h01 - h00) * fractX + h00;
        const float topH    = (h11 - h10) * fractX + h10;

        const float finalH = (topH - bottomH) * fractY + bottomH;

        return finalH;
    }

    float HeightMap::getLevel(const glm::vec3& worldPos) const noexcept
    {
        const auto& transform = m_origin->getTransform();
        const auto& originPos = transform.getWorldPosition();
        const auto& modelMat = transform.getModelMatrix();

        const auto diff = worldPos - originPos;

        const float u = diff.x / (float)m_worldSizeU;
        const float v = 1.f - diff.z / (float)m_worldSizeV;

        const float h = getTerrainHeight(u, v);

        const auto p = glm::vec4{ 0.f, h, 0.f, 1.f };

        const auto newWorldPos = modelMat * p;
        return newWorldPos.y;
    }
}
