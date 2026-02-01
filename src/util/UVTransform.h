#pragma once

#include <glm/glm.hpp>

#include "ki/size.h"

#include "glm_util.h"

namespace util
{
    struct UVTransform
    {
        glm::vec2 m_position{ 0.f };
        glm::vec2 m_scale{ 1.f };
        // radians 
        float m_rotation{ 0.f };

        inline void setScale(float scale) noexcept
        {
            m_scale.x = scale;
            m_scale.y = scale;
        }

        inline void setDegreesRotation(const float& degrees) noexcept
        {
            m_rotation = glm::radians(degrees);
        }

        inline glm::mat3 toMatrix() const noexcept
        {
            float cosR = std::cos(m_rotation);
            float sinR = std::sin(m_rotation);

            // Scale -> Rotate -> Translate
            // Combined into a single mat3:
            //
            // | sx*cos  -sy*sin  tx |
            // | sx*sin   sy*cos  ty |
            // |   0        0      1 |

            return glm::mat3{
                m_scale.x * cosR, m_scale.x * sinR, 0.f,   // column 0
                -m_scale.y * sinR, m_scale.y * cosR, 0.f,   // column 1
                m_position.x, m_position.y, 1.f    // column 2
            };
        }

        inline glm::vec2 transformUV(const glm::vec2& uv) const noexcept
        {
            glm::vec3 result = toMatrix() * glm::vec3(uv, 1.f);
            return glm::vec2(result);
        }
    };
}
