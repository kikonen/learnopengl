#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

namespace mesh {
    struct MeshTransform {
        glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };
        float m_x{ 0.f };
        float m_y{ 0.f };
        float m_z{ 0.f };
        float m_scale{ 1.f };
        glm::vec4 m_volume{ 0.f };
        glm::mat4 m_transform{ 1.f };

        //glm::vec4 m_worldPos2{ 1.f };
        //glm::mat4 m_parentMatrix{ 1.f };

        inline glm::vec3 getPosition() const noexcept
        {
            return glm::vec3{ m_x, m_y, m_z };
        }

        inline void setPosition(const glm::vec3& pos)
        {
            m_x = pos.x;
            m_y = pos.y;
            m_z = pos.z;
        }

        inline void setScale(const float scale)
        {
            m_scale = scale;
        }

        inline void adjustRotation(const glm::quat& adjust) noexcept
        {
            setRotation(adjust * m_rotation);
        }

        inline void setDegreesRotation(const glm::vec3& degrees)
        {
            setRotation(glm::quat(glm::radians(degrees)));
        }

        inline void setRotation(const glm::quat& rotation)
        {
            m_rotation = rotation;
        }

        inline const glm::vec4& getVolume() const noexcept
        {
            return m_volume;
        }

        inline void setVolume(const glm::vec4& volume)
        {
            m_volume = volume;
        }

        inline glm::vec3 getWorldPosition() const noexcept
        {
            return glm::vec3{ m_volume };
        }

        inline const glm::mat4& getTransform() const noexcept {
            return m_transform;
        }

        inline void updateTransform(const glm::mat4& parentMatrix) noexcept
        {
            m_transform = glm::translate(glm::mat4{ 1.f }, getPosition()) *
                glm::mat4{ m_rotation } *
                glm::scale(glm::mat4{ 1.f }, glm::vec3{ m_scale });

            const auto& worldPos = (parentMatrix * m_transform)[3];
            m_volume.x = worldPos.x;
            m_volume.y = worldPos.y;
            m_volume.z = worldPos.z;

            //m_parentMatrix = parentMatrix;
            //m_worldPos2 = m_transform[3];
        }
    };
}
