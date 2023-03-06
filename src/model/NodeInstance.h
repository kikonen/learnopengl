#pragma once

#include <glm/glm.hpp>

struct EntitySSBO;

//
// Relative to *Node*
//
// NOTE KI purposee of this struct is to separate
// rendering entity and node instance updated, with aim to use separate threads
//
struct NodeInstance {
    bool m_dirty = true;
    bool m_entityDirty = true;

    int m_parentMatrixLevel = -1;
    int m_matrixLevel = -1;

    int m_physicsMatrixLevel = -1;
    int m_physicsLevel = -1;

    int m_objectID{ 0 };
    int m_entityIndex{ -1 };
    int m_flags = { 0 };

    int m_materialIndex{ 0 };

    glm::vec4 m_volume{ 0.f };

    glm::mat4 m_translateMatrix{ 1.f };
    glm::mat4 m_scaleMatrix{ 1.f };

    glm::vec3 m_rotation{ 0.f };
    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    // quaternion rotation matrix
    glm::mat4 m_rotationMatrix{ 1.f };

    glm::mat4 m_modelMatrix{ 1.f };

    inline int getObjectID() const noexcept {
        return m_objectID;
    }

    inline void setObjectID(int objectID) noexcept {
        m_objectID = objectID;
    }

    inline int getFlags() const noexcept {
        return m_flags;
    }

    inline void setFlags(int flags) noexcept {
        if (m_flags != flags) {
            m_flags = flags;
            m_entityDirty = true;
        }
    }

    inline int getMaterialIndex() const noexcept {
        return m_materialIndex;
    }

    inline void setMaterialIndex(int materialIndex) noexcept {
        if (m_materialIndex != materialIndex) {
            m_materialIndex = materialIndex;
            m_entityDirty = true;
        }
    }

    inline const glm::vec4& getVolume() const noexcept {
        return m_volume;
    }

    inline void setVolume(const glm::vec4& volume) noexcept {
        if (m_volume != volume) {
            m_volume = volume;
            m_entityDirty = true;
        }
    }

    inline const glm::vec3 getPosition() const noexcept {
        return { m_translateMatrix[3][0], m_translateMatrix[3][1], m_translateMatrix[3][2] };
    }

    inline const glm::vec3 getScale() const noexcept {
        return { m_scaleMatrix[0][0], m_scaleMatrix[1][1], m_scaleMatrix[2][2] };
    }

    inline const glm::vec3& getRotation() const noexcept {
        return m_rotation;
    }

    inline void setPosition(const glm::vec3& pos) noexcept
    {
        auto& vec = m_translateMatrix[3];
        if (vec[0] != pos.x ||
            vec[1] != pos.y ||
            vec[2] != pos.z)
        {
            vec[0] = pos.x;
            vec[1] = pos.y;
            vec[2] = pos.z;
            m_dirty = true;
        }
    }

    inline void setScale(float scale) noexcept
    {
        assert(scale >= 0);
        if (m_scaleMatrix[0][0] != scale ||
            m_scaleMatrix[1][1] != scale ||
            m_scaleMatrix[2][2] != scale)
        {
            m_scaleMatrix[0][0] = scale;
            m_scaleMatrix[1][1] = scale;
            m_scaleMatrix[2][2] = scale;
            m_dirty = true;
        }
    }

    inline void setScale(const glm::vec3& scale) noexcept
    {
        assert(scale.x >= 0 && scale.y >= 0 && scale.z >= 0);
        if (m_scaleMatrix[0][0] != scale.x||
            m_scaleMatrix[1][1] != scale.y ||
            m_scaleMatrix[2][2] != scale.z)
        {
            m_scaleMatrix[0][0] = scale.x;
            m_scaleMatrix[1][1] = scale.y;
            m_scaleMatrix[2][2] = scale.z;
            m_dirty = true;
        }
    }

    void setRotation(const glm::vec3& rotation) noexcept;

    inline const glm::vec3 getWorldPosition() const noexcept {
        return m_modelMatrix[3];
    }

    inline void updateRootMatrix() noexcept
    {
        if (!m_dirty) return;

        m_modelMatrix = m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
        m_dirty = false;
        m_matrixLevel++;
        m_entityDirty = true;
    }

    inline void updateModelMatrix(const glm::mat4& parentMatrix, int parentMatrixLevel) noexcept
    {
        if (!m_dirty && parentMatrixLevel == m_parentMatrixLevel) return;

        m_modelMatrix = parentMatrix * m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
        m_dirty = false;
        m_parentMatrixLevel = parentMatrixLevel;
        m_matrixLevel++;
        m_entityDirty = true;
    }

    void updateEntity(EntitySSBO* entity);
};
