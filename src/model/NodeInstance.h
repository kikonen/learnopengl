#pragma once

#include <glm/glm.hpp>


// Relative to *Node*
struct NodeInstance {
    int m_entityIndex{ 0 };

    glm::mat4 m_translateMatrix{ 1.f };
    glm::mat4 m_scaleMatrix{ 1.f };

    glm::vec3 m_rotation{ 0.f };
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    // quaternion rotation matrix
    glm::mat4 m_rotationMatrix{ 1.f };

    inline const glm::vec3& getPosition() const noexcept {
        return { m_translateMatrix[3][0], m_translateMatrix[3][1], m_translateMatrix[3][2] };
    }

    inline const glm::vec3& getScale() const noexcept {
        return { m_scaleMatrix[0][0], m_scaleMatrix[1][1], m_scaleMatrix[2][2] };
    }

    inline const glm::vec3& getRotation() const noexcept {
        return m_rotation;
    }

    inline void setPosition(const glm::vec3& pos) noexcept
    {
        m_translateMatrix[3][0] = pos.x;
        m_translateMatrix[3][1] = pos.y;
        m_translateMatrix[3][2] = pos.z;
    }

    inline void setScale(const glm::vec3& scale) noexcept
    {
        assert(scale.x >= 0 && scale.y >= 0 && scale.z >= 0);
        m_scaleMatrix[0][0] = scale.x;
        m_scaleMatrix[1][1] = scale.y;
        m_scaleMatrix[2][2] = scale.z;
    }

    void setRotation(const glm::vec3& rotation) noexcept;

    inline const glm::mat4& compile(const glm::mat4& parentMatrix) const
    {
        return parentMatrix * m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
    }
};
