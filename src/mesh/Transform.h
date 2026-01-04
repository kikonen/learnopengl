#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/SphereVolume.h"

#include "ki/size.h"

namespace mesh {
    // Local transform to position mesh within Node
    struct Transform {
    private:
        glm::mat4 m_matrix{ 1.f };
        glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };
        float m_x{ 0.f };
        float m_y{ 0.f };
        float m_z{ 0.f };

        float m_scaleX{ 1.f };
        float m_scaleY{ 1.f };
        float m_scaleZ{ 1.f };

        float m_worldPosX{ 0.f };
        float m_worldPosY{ 0.f };
        float m_worldPosZ{ 0.f };

        // NOTE KI volume center is not same as worldPos always
        SphereVolume m_worldVolume{};

        uint32_t m_data{ 0 };

    public:
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

        inline glm::vec3 getScale() const noexcept
        {
            return glm::vec3{ m_scaleX, m_scaleY, m_scaleZ };
        }

        inline void setScale(const glm::vec3& scale) noexcept
        {
            m_scaleX = scale.x;
            m_scaleY = scale.y;
            m_scaleZ = scale.z;
        }

        inline void setScale(float scale) noexcept
        {
            m_scaleX = scale;
            m_scaleY = scale;
            m_scaleZ = scale;
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

        inline const glm::quat& getRotation() const noexcept
        {
            return m_rotation;
        }

        inline const SphereVolume& getWorldVolume() const noexcept
        {
            return m_worldVolume;
        }

        inline void setWorldVolume(const SphereVolume& worldVolume) noexcept
        {
            m_worldVolume = worldVolume;
        }

        inline glm::vec3 getWorldPosition() const noexcept
        {
            return glm::vec3{ m_worldPosX, m_worldPosY, m_worldPosZ };
        }

        inline uint32_t getData() const noexcept
        {
            return m_data;
        }

        inline void setData(uint32_t data) noexcept
        {
            m_data = data;
        }

        inline const glm::mat4& getMatrix() const noexcept
        {
            return m_matrix;
        }

        // @param volume non-scaled (will be scaled using scale)
        inline void updateMatrix() noexcept
        {
            m_matrix = glm::translate(glm::mat4{ 1.f }, getPosition()) *
                glm::mat4{ m_rotation } *
                glm::scale(glm::mat4{ 1.f }, glm::vec3{ m_scaleX, m_scaleY, m_scaleZ });
        }

        inline void updateWorldVolume() noexcept
        {
            const glm::mat4 ID_MAT{ 1.f };
            const SphereVolume ID_VOLUME{ 0.f, 0.f, 0.f, 1.f };
            updateWorldVolume(ID_MAT, ID_VOLUME);
        }

        inline void updateWorldVolume(
            const glm::mat4& parentMatrix,
            const SphereVolume& localVolume) noexcept
        {
            const auto& modelMatrix = parentMatrix * m_matrix;
            {
                const auto& wp = modelMatrix[3];
                m_worldPosX = wp.x;
                m_worldPosY = wp.y;
                m_worldPosZ = wp.z;
            }
            {
                auto v = modelMatrix * glm::vec4{ localVolume.x, localVolume.y, localVolume.z, 1.f };

                m_worldVolume = v;
                m_worldVolume.radius = localVolume.radius * std::max(m_scaleX, std::max(m_scaleY, m_scaleZ));
            }
        }
    };
}
