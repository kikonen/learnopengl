#pragma once

#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/limits.h"
#include "ki/size.h"

#include "util/glm_util.h"


namespace model
{
    struct Snapshot;
}

namespace render
{
    class RenderContext;
}

struct UpdateContext;

namespace model
{
    //
    // Relative to *Node*
    //
    // NOTE KI purposee of this struct is to separate
    // rendering entity and node instance updated, with aim to use separate threads
    //
    struct NodeState {
        friend struct Snapshot;

    private:
        glm::vec4 m_volume;

        glm::vec3 m_position{ 0.f };

        glm::vec3 m_scale{ 1.f };
        glm::vec3 m_baseScale{ 1.f };

        glm::vec3 m_pivotAlignment{ 0.f };
        glm::vec3 m_pivotOffset{ 0.f };

        glm::vec3 m_worldPivot{ 0.f };

        // Base rotation for node
        glm::quat m_baseRotation{ 1.f, 0.f, 0.f, 0.f };
        glm::quat m_invBaseRotation{ 1.f, 0.f, 0.f, 0.f };

        // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
        glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };

        glm::vec3 m_up{ 0.f, 1.f, 0.f };
        glm::vec3 m_front{ 0.f, 0.f, 1.f };

        mutable glm::vec3 m_viewUp{ 0.f };
        mutable glm::vec3 m_viewFront{ 0.f };

        glm::mat4 m_modelMatrix{ 1.f };
        glm::vec3 m_modelScale{ 1.f };
        glm::quat m_modelRotation{ 1.f, 0.f, 0.f, 0.f };

        glm::uvec2 m_aspectRatio{ 1 };

    public:
        float m_tilingX{ 1.f };
        float m_tilingY{ 1.f };

        // parent socket
        uint32_t m_attachedSocketIndex{ 0 };

        ki::tag_id m_tagId{ 0 };

        ki::size_t_entity_flags m_flags{ 0 }; // 1 * 4 = 4

        ki::level_id m_parentMatrixLevel{ 0 };
        ki::level_id m_matrixLevel{ 0 };

        bool m_dirty : 1 {true};

        mutable bool m_dirtyNormal : 1 {true};
        mutable bool m_dirtySnapshot : 1 {true};

        bool boundStaticDone : 1 { false };

        ///////////////////////////////////////
        //
    public:
        inline bool valid(ki::level_id parentMatrixLevel) const noexcept
        {
            return !m_dirty && parentMatrixLevel == m_parentMatrixLevel;
        }

        void setBaseRotation(const glm::quat& rotation) noexcept
        {
            if (m_baseRotation != rotation) {
                m_baseRotation = rotation;
                m_invBaseRotation = glm::conjugate(rotation);
                m_dirty = true;
                m_dirtySnapshot = true;
            }
        }

        inline const glm::quat& getBaseRotation() const noexcept
        {
            return m_baseRotation;
        }

        inline const glm::quat& getInvBaseRotation() const noexcept
        {
            return m_invBaseRotation;
        }

        inline const glm::vec4& getVolume() const noexcept
        {
            return m_volume;
        }

        inline void setVolume(const glm::vec4& volume) noexcept
        {
            if (m_volume != volume) {
                m_volume = volume;
                m_dirtySnapshot = true;
            }
        }

        inline const glm::vec3& getPosition() const noexcept
        {
            return m_position;
        }

        inline const glm::vec3& getScale() const noexcept
        {
            return m_scale;
        }

        inline const glm::quat& getRotation() const noexcept
        {
            return m_rotation;
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

        inline void setScale(float scale) noexcept
        {
            setScale({ scale, scale, scale });
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

        inline const glm::vec3& getPivotAlignment() const noexcept
        {
            return m_pivotAlignment;
        }

        inline void setPivotAlignment(
            const glm::vec3& pivotAlignment) noexcept
        {
            if (m_pivotAlignment != pivotAlignment)
            {
                m_pivotAlignment = pivotAlignment;
                m_dirty = true;
            }
        }

        inline const glm::vec3& getPivotOffset() const noexcept
        {
            return m_pivotOffset;
        }

        inline void setPivotOffset(
            const glm::vec3& pivotOffset) noexcept
        {
            if (m_pivotOffset != pivotOffset)
            {
                m_pivotOffset = pivotOffset;
                m_dirty = true;
            }
        }

        inline void adjustScale(const glm::vec3& adjust) noexcept
        {
            if (adjust.x == 0 && adjust.y == 0 && adjust.z == 0) return;
            //assert(m_scale.x + adjust.x >= 0 && m_scale.y + adjust.y >= 0 && m_scale.z + adjust.z >= 0);

            m_scale.x += adjust.x;
            m_scale.y += adjust.y;
            m_scale.z += adjust.z;

            if (m_scale.x < 0) m_scale.x = 0;
            if (m_scale.y < 0) m_scale.y = 0;
            if (m_scale.z < 0) m_scale.z = 0;

            m_dirty = true;
            m_dirtyNormal = true;
        }

        inline void setAspectRatio(const glm::uvec2& aspectRatio) noexcept
        {
            if (m_aspectRatio != aspectRatio) {
                m_aspectRatio = aspectRatio;

                m_dirty = true;
                m_dirtyNormal = true;
            }
        }

        void setRotation(const glm::quat& quat) noexcept
        {
            if (m_rotation != quat) {
                m_rotation = quat;
                m_dirty = true;
                m_dirtyNormal = true;
            }
        }

        inline void adjustRotation(const glm::quat& adjust) noexcept
        {
            setRotation(adjust * m_rotation);
        }

        glm::vec3 getDegreesRotation() const noexcept;

        void setDegreesRotation(const glm::vec3& rot) noexcept
        {
            setRotation(glm::quat(glm::radians(rot)));
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
            //updateModelAxis();
            return m_viewUp;
        }

        inline const glm::vec3& getViewFront() const noexcept {
            assert(!m_dirty);
            //updateModelAxis();
            return m_viewFront;
        }

        inline glm::vec3 getViewRight() const noexcept {
            //updateModelAxis();
            return glm::cross(m_viewFront, m_viewUp);
        }

        inline const glm::vec3& getWorldPivot() const noexcept
        {
            // TODO KI sync with physics
            //assert(!m_dirty);
            return m_worldPivot;
        }

        inline glm::vec3 getWorldPosition() const noexcept
        {
            // TODO KI sync with physics
            //assert(!m_dirty);
            //return m_worldPos;
            return m_modelMatrix[3];
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

        inline const glm::quat& getModelRotation() const noexcept
        {
            assert(!m_dirty);
            return m_modelRotation;
        }

        void updateRootMatrix() noexcept;
        void updateModelMatrix(const NodeState& parent) noexcept;
        void updateModelAxis() const noexcept;
    };
}
