#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/size.h"

#include "glm_util.h"

namespace util {
    struct Transform {
        glm::vec3 m_position{ 0.f };
        glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };
        glm::vec3 m_scale{ 1.f };

        inline void setScale(float scale) noexcept
        {
            m_scale.x = scale;
            m_scale.y = scale;
            m_scale.z = scale;
        }

        inline void adjustRotation(const glm::quat& adjust) noexcept
        {
            m_rotation = adjust * m_rotation;
        }

        inline void setDegreesRotation(const glm::vec3& degrees) noexcept
        {
            m_rotation = glm::quat(glm::radians(degrees));
        }

        inline glm::mat4 toMatrix() const noexcept
        {
            return glm::translate(glm::mat4{ 1.f }, m_position) *
                glm::mat4{ m_rotation } *
                glm::scale(glm::mat4{ 1.f }, m_scale);
        }
    };
}
