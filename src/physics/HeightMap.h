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

        // @param pz [0, 1]
        // @param px [0, 1]
        float getHeight(float z, float x);

    public:
        const std::unique_ptr<Image> m_image;

        glm::vec2 m_verticalRange{ 0.f, 32.f };
        float m_horizontalScale{ 1.f };

    private:
        float* m_heights{ nullptr };
    };

}
