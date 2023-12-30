#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/Sphere.h"

#include "ki/size.h"

#include "util/glm_util.h"


class UpdateContext;
class RenderContext;

struct EntitySSBO;

//
// Relative to *Node*
//
// NOTE KI purposee of this struct is to separate
// rendering entity and node instance updated, with aim to use separate threads
//
struct NodeTransform {
    bool m_dirty : 1 {true};
    bool m_dirtyRotation : 1 {true};
    mutable bool m_dirtyDegrees : 1 {true};
    bool m_dirtyEntity : 1 {true};
    bool m_dirtySnapshot : 1 {true};
    bool m_uniformScale : 1 {false};

    ki::level_id m_parentMatrixLevel{ (ki::level_id)-1 };
    ki::level_id m_matrixLevel{ (ki::level_id)-1 };

    ki::level_id m_physicsLevel{ (ki::level_id)-1 };

    int m_entityIndex{ -1 };

    int m_materialIndex{ 0 };
    int m_shapeIndex{ 0 };

    Sphere m_volume;

    glm::vec3 m_position{ 0.f };
    glm::vec3 m_scale{ 1.f };

    glm::vec3 m_worldPos{ 0.f };

    // Rotation for geometry, to align it correct way
    // i.e. not affecting "front"
    glm::quat m_baseRotation{ 1.f, 0.f, 0.f, 0.f };

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    glm::quat m_quatRotation{ 1.f, 0.f, 0.f, 0.f };
    mutable glm::vec3 m_degreesRotation{ 0.f };
    glm::mat4 m_rotationMatrix{ 1.f };

    glm::vec3 m_up{ 0.f, 1.f, 0.f };
    glm::vec3 m_front{ 0.f, 0.f, 1.f };

    glm::vec3 m_viewUp{ 0.f };
    glm::vec3 m_viewFront{ 0.f };
    glm::vec3 m_viewRight{ 0.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::vec3 m_modelScale{ 1.f };

    inline int getMaterialIndex() const noexcept
    {
        return m_materialIndex;
    }

    inline void setMaterialIndex(int materialIndex) noexcept
    {
        if (m_materialIndex != materialIndex) {
            m_materialIndex = materialIndex;
            m_dirtyEntity = true;
            m_dirtySnapshot = true;
        }
    }

    inline const glm::vec4 getVolume() const noexcept
    {
        return m_volume.getVolume();
    }

    inline void setVolume(const glm::vec4& volume) noexcept
    {
        if (m_volume.getVolume() != volume) {
            m_volume = volume;
            m_dirtyEntity = true;
            m_dirtySnapshot = true;
        }
    }

    inline const glm::vec3 getPosition() const noexcept
    {
        return m_position;;
    }

    inline const bool isUniformScale() const noexcept
    {
        return m_uniformScale;
    }

    inline const glm::vec3& getScale() const noexcept
    {
        return m_scale;
    }

    inline const glm::vec3& getDegreesRotation() const noexcept
    {
        updateDegrees();
        return m_degreesRotation;
    }

    inline const glm::quat& getQuatRotation() const noexcept
    {
        return m_quatRotation;
    }

    inline void setPosition(const glm::vec3& pos) noexcept
    {
        if (m_position != pos) {
            m_position = pos;
            m_dirty = true;
        }
    }

    inline void adjustPosition(const glm::vec3& adjust) noexcept
    {
        if (adjust.x == 0 && adjust.y == 0 && adjust.z == 0) return;
        m_position += adjust;
        m_dirty = true;
    }

    inline void setScale(float scale) noexcept
    {
        assert(scale >= 0);
        if (m_scale.x != scale || m_scale.y != scale || m_scale.z != scale) {
            m_scale.x = scale;
            m_scale.y = scale;
            m_scale.z = scale;

            m_uniformScale = m_scale.x == m_scale.y && m_scale.x == m_scale.z;
            m_dirty = true;
        }
    }

    inline void setScale(const glm::vec3& scale) noexcept
    {
        assert(scale.x >= 0 && scale.y >= 0 && scale.z >= 0);

        if (m_scale != scale)
        {
            m_scale.x = scale.x;
            m_scale.y = scale.y;
            m_scale.z = scale.z;

            m_uniformScale = m_scale.x == m_scale.y && m_scale.x == m_scale.z;
            m_dirty = true;
        }
    }

    inline void adjustScale(const glm::vec3& adjust) noexcept
    {
        if (adjust.x == 0 && adjust.y == 0 && adjust.z == 0) return;
        assert(m_scale.x + adjust.x >= 0 && m_scale.y + adjust.y >= 0 && m_scale.z + adjust.z >= 0);

        m_scale.x += adjust.x;
        m_scale.y += adjust.y;
        m_scale.z += adjust.z;

        m_uniformScale = m_scale.x == m_scale.y && m_scale.x == m_scale.z;
        m_dirty = true;
    }

    void setBaseRotation(const glm::quat& quat) noexcept
    {
        m_baseRotation = glm::normalize(quat);
        m_dirtyRotation = true;
        m_dirtyDegrees = true;
        m_dirty = true;
    }

    void setQuatRotation(const glm::quat& quat) noexcept
    {
        if (m_quatRotation != quat) {
            m_quatRotation = quat;
            m_dirtyRotation = true;
            m_dirtyDegrees = true;
            m_dirty = true;
        }
    }

    inline void adjustQuatRotation(const glm::quat& adjust) noexcept
    {
        setQuatRotation(adjust * m_quatRotation);
    }

    void setDegreesRotation(const glm::vec3& rot) noexcept
    {
        setQuatRotation(glm::quat(glm::radians(rot)));
    }

    inline void adjustDegreesRotation(const glm::vec3& adjust) noexcept
    {
        glm::vec3 rot{ adjust };
        {
            rot.x += m_degreesRotation.x;
            rot.y += m_degreesRotation.y;
            rot.z += m_degreesRotation.z;
        }
        setDegreesRotation(rot);
    }

    inline const glm::vec3& getFront() const noexcept {
        return m_front;
    }

    void setFront(const glm::vec3& front) noexcept
    {
        if (m_front != front) {
            m_front = front;
            m_dirty = true;
        }
    }

    inline const glm::vec3& getViewUp() const noexcept {
        return m_viewUp;
    }

    inline const glm::vec3& getViewFront() const noexcept {
        return m_viewFront;
    }

    inline const glm::vec3& getViewRight() const noexcept {
        return m_viewRight;
    }

    inline const glm::vec3& getWorldPosition() const noexcept
    {
        return m_worldPos;
    }

    inline const glm::vec3& getWorldScale() const noexcept
    {
        return m_modelScale;
    }

    inline float getWorldMaxScale() const noexcept
    {
        return std::max(std::max(m_modelScale.x, m_modelScale.y), m_modelScale.z);
    }

    inline ki::level_id getParentMatrixLevel() const noexcept {
        return m_parentMatrixLevel;
    }

    inline ki::level_id getMatrixLevel() const noexcept {
        return m_matrixLevel;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_modelMatrix;
    }

    void updateRootMatrix() noexcept;
    void updateDegrees() const noexcept;
    void updateModelMatrix(const NodeTransform& parent) noexcept;
    void updateModelAxis() noexcept;
    void updateRotationMatrix() noexcept;
};
