#pragma once

#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/Sphere.h"

#include "ki/limits.h"
#include "ki/size.h"

#include "util/glm_util.h"


struct UpdateContext;
class RenderContext;

//
// Relative to *Node*
//
// NOTE KI purposee of this struct is to separate
// rendering entity and node instance updated, with aim to use separate threads
//
struct NodeTransform {
    Sphere m_volume;

    glm::vec3 m_position{ 0.f };
    glm::vec3 m_scale{ 1.f };
    // Basescale, normalizes scale
    glm::vec3 m_baseScale{ 1.f };

    glm::vec3 m_worldPos{ 0.f };

    // Base transform for mesh
    glm::mat4 m_baseTransform{ 1.f };

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    glm::quat m_quatRotation{ 1.f, 0.f, 0.f, 0.f };
    glm::mat4 m_rotationMatrix{ 1.f };

    glm::vec3 m_up{ 0.f, 1.f, 0.f };
    glm::vec3 m_front{ 0.f, 0.f, 1.f };

    glm::vec3 m_viewUp{ 0.f };
    glm::vec3 m_viewFront{ 0.f };
    //glm::vec3 m_viewRight{ 0.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::vec3 m_modelScale{ 1.f };

    //std::array<int32_t, ki::MAX_LOD> m_lodMaterialIndeces;

    uint32_t m_shapeIndex{ 0 };
    uint32_t m_boneIndex{ 0 };
    uint32_t m_socketIndex{ 0 };

    uint16_t m_animationIndex{ 0 };
    double m_animationStartTime{ -1.f };

    ki::size_t_entity_flags m_flags{ 0 }; // 1 * 4 = 4

    ki::level_id m_parentMatrixLevel{ 0 };
    ki::level_id m_matrixLevel{ 0 };

    ki::level_id m_physicsLevel{ 0 };

    bool m_dirty : 1 {true};
    bool m_dirtyRotation : 1 {true};

    mutable bool m_dirtyNormal : 1 {true};
    mutable bool m_dirtySnapshot : 1 {true};

    ///////////////////////////////////////
    //

    void setBaseTransform(const glm::mat4& transform) noexcept
    {
        if (m_baseTransform != transform) {
            m_baseTransform = transform;
            m_dirty = true;
            m_dirtyRotation = true;
            m_dirtyNormal = true;
            m_dirtySnapshot = true;
        }
    }

    inline const glm::mat4& getBaseTransform() const noexcept
    {
        return m_baseTransform;
    }

    inline const glm::vec4 getVolume() const noexcept
    {
        return m_volume.getVolume();
    }

    inline void setVolume(const glm::vec4& volume) noexcept
    {
        if (m_volume.getVolume() != volume) {
            m_volume = volume;
            m_dirtySnapshot = true;
        }
    }

    inline const glm::vec3 getPosition() const noexcept
    {
        return m_position;;
    }

    inline const glm::vec3& getScale() const noexcept
    {
        return m_scale;
    }

    inline const glm::vec3& getBaseScale() const noexcept
    {
        return m_baseScale;
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
        setScale({ scale, scale, scale });
    }

    inline void setBaseScale(float scale) noexcept
    {
        setBaseScale({ scale, scale, scale });
    }

    inline void setScale(const glm::vec3& scale) noexcept
    {
        assert(scale.x >= 0 && scale.y >= 0 && scale.z >= 0);

        if (m_scale != scale)
        {
            m_scale.x = scale.x;
            m_scale.y = scale.y;
            m_scale.z = scale.z;

            m_dirty = true;
            m_dirtyNormal = true;
        }
    }

    inline void setBaseScale(const glm::vec3& scale) noexcept
    {
        assert(scale.x >= 0 && scale.y >= 0 && scale.z >= 0);

        if (m_baseScale != scale)
        {
            m_baseScale.x = scale.x;
            m_baseScale.y = scale.y;
            m_baseScale.z = scale.z;

            m_dirty = true;
            m_dirtyNormal = true;
        }
    }

    inline void adjustScale(const glm::vec3& adjust) noexcept
    {
        if (adjust.x == 0 && adjust.y == 0 && adjust.z == 0) return;
        assert(m_scale.x + adjust.x >= 0 && m_scale.y + adjust.y >= 0 && m_scale.z + adjust.z >= 0);

        m_scale.x += adjust.x;
        m_scale.y += adjust.y;
        m_scale.z += adjust.z;

        m_dirty = true;
        m_dirtyNormal = true;
    }

    void setQuatRotation(const glm::quat& quat) noexcept
    {
        if (m_quatRotation != quat) {
            m_quatRotation = quat;
            m_dirtyRotation = true;
            m_dirty = true;
            m_dirtyNormal = true;
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
        assert(!m_dirty);
        return m_viewUp;
    }

    inline const glm::vec3& getViewFront() const noexcept {
        assert(!m_dirty);
        return m_viewFront;
    }

    //inline const glm::vec3& getViewRight() const noexcept {
    //    assert(!m_dirty);
    //    return m_viewRight;
    //}

    inline const glm::vec3& getWorldPosition() const noexcept
    {
        // TODO KI sync with physics
        //assert(!m_dirty);
        return m_worldPos;
    }

    inline const glm::vec3& getWorldScale() const noexcept
    {
        assert(!m_dirty);
        return m_modelScale;
    }

    inline float getWorldMaxScale() const noexcept
    {
        // TODO KI sync with physics
        //assert(!m_dirty);
        return std::max(std::max(m_modelScale.x, m_modelScale.y), m_modelScale.z);
    }

    inline ki::level_id getParentMatrixLevel() const noexcept {
        return m_parentMatrixLevel;
    }

    inline ki::level_id getMatrixLevel() const noexcept {
        return m_matrixLevel;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        assert(!m_dirty);
        return m_modelMatrix;
    }

    void updateRootMatrix() noexcept;
    void updateModelMatrix(const NodeTransform& parent) noexcept;
    void updateModelAxis() noexcept;
    void updateRotationMatrix() noexcept;
};
