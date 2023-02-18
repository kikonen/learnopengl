#pragma once

#include <glm/glm.hpp>

#include "asset/Image.h"

#include "Surface.h"

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

        glm::vec3 m_origin{ 0.f };

        glm::vec2 m_verticalRange{ 0.f, 32.f };
        float m_horizontalScale{ 1.f };

    private:
        int m_height{ 0 };
        int m_width{ 0 };

        float* m_heights{ nullptr };
    };

}
