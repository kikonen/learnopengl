#pragma once

#include <glm/glm.hpp>

#include "Surface.h"

class Image;
class Node;

namespace physics {
    class HeightMap : public Surface {
    public:
        HeightMap(
            std::unique_ptr<Image> image)
            : m_image(std::move(image))
        {
        }

        ~HeightMap()
        {
            delete[] m_heights;
        }

        void prepare();

        virtual float getLevel(const glm::vec3& pos) override;

        // Using texture coordinates
        //
        // @param u [0, 1]
        // @param v [0, 1]
        float getTerrainHeight(float u, float v);

    public:
        const std::unique_ptr<Image> m_image;

        Node* m_origin{ nullptr };

        int m_worldTileSize{ 0 };
        int m_worldSizeU{ 0 };
        int m_worldSizeV{ 0 };

        glm::vec2 m_verticalRange{ 0.f, 32.f };
        float m_horizontalScale{ 1.f };

    private:
        int m_height{ 0 };
        int m_width{ 0 };

        float* m_heights{ nullptr };
    };

}
