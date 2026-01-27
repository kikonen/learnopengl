#include "NodeState.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "util/thread.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

#include "animation/AnimationSystem.h"

namespace {
    const glm::vec3 ZERO_VEC{ 0.f };

    const glm::mat4 ID_MAT{ 1.f };

    // NOTE KI only *SINGLE* thread is allowed to do model updates
    // => thus can use globally shared temp vars
    static glm::mat4 g_translateMatrix{ 1.f };
    static glm::mat4 g_scaleMatrix{ 1.f };
    static glm::mat4 g_pivotMatrix{ 1.f };
    static glm::mat4 g_invPivotMatrix{ 1.f };
}

namespace model
{
    glm::vec3 NodeState::getDegreesRotation() const noexcept
    {
        return util::quatToDegrees(m_rotation);
    }

    void NodeState::updateRootMatrix() noexcept
    {
        //ASSERT_WT();
        if (!m_dirty) return;

        const auto& rotationMatrix = glm::toMat4(m_rotation * m_baseRotation);

        static glm::mat4 s_translateMatrix{ 1.f };
        static glm::mat4 s_scaleMatrix{ 1.f };
        {
            s_translateMatrix[3].x = m_position.x;
            s_translateMatrix[3].y = m_position.y;
            s_translateMatrix[3].z = m_position.z;

            s_scaleMatrix[0].x = m_scale.x * m_baseScale.x;
            s_scaleMatrix[1].y = m_scale.y * m_baseScale.y;
            s_scaleMatrix[2].z = m_scale.z * m_baseScale.z;
        }

        m_modelMatrix = s_translateMatrix * rotationMatrix * s_scaleMatrix;
        m_modelScale = m_scale * m_baseScale;

        updateModelAxis();

        m_matrixLevel++;

        m_dirty = false;
        m_dirtySnapshot = true;
    }

    void NodeState::updateModelMatrix(const NodeState& parent) noexcept
    {
        ASSERT_WT();

        // TODO KI should check if parent is animated
        m_dirty |= m_attachedSocketIndex > 0;

        if (!m_dirty && parent.m_matrixLevel == m_parentMatrixLevel) return;

        const float aspect = (float)m_aspectRatio.x / (float)m_aspectRatio.y;
        const float aspectScaleX = m_scale.x * m_baseScale.x;// / aspect;
        const float aspectScaleY = m_scale.y * m_baseScale.y;

        const auto hasPivot = m_pivotAlignment != ZERO_VEC || m_pivotOffset != ZERO_VEC;

        {
            g_translateMatrix[3].x = m_position.x * aspect;
            g_translateMatrix[3].y = m_position.y;
            g_translateMatrix[3].z = m_position.z;

            g_scaleMatrix[0].x = aspectScaleX;
            g_scaleMatrix[1].y = aspectScaleY;
            g_scaleMatrix[2].z = m_scale.z * m_baseScale.z;

            if (hasPivot) {
                const auto pivot = m_pivotAlignment + m_pivotOffset;

                g_pivotMatrix[3].x = -pivot.x * aspectScaleX;
                g_pivotMatrix[3].y = -pivot.y * aspectScaleY;
                g_pivotMatrix[3].z = -pivot.z * m_scale.z * m_baseScale.z;

                g_invPivotMatrix[3].x = pivot.x * aspectScaleX;
                g_invPivotMatrix[3].y = pivot.y * aspectScaleY;
                g_invPivotMatrix[3].z = pivot.z * m_scale.z * m_baseScale.z;
            }
        }

        auto rotationMatrix = glm::toMat4(m_rotation * m_baseRotation);
        if (hasPivot) {
            rotationMatrix = g_invPivotMatrix *
                rotationMatrix *
                g_pivotMatrix;
        }

        const auto& parentModelMatrix = parent.m_modelMatrix;


        {
            //m_modelMatrix = parentModelMatrix *
            //    g_translateMatrix *
            //    rotationMatrix *
            //    g_scaleMatrix;
            glm::vec3 scale = m_scale * m_baseScale.z;
            scale.x = aspectScaleX;
            scale.y = aspectScaleY;

            if (m_attachedSocketIndex) {
                // Socket provides coordinate system, node transform is relative to socket
                const auto& socketTransform = animation::AnimationSystem::get().getSocketTransform(m_attachedSocketIndex);
                m_modelMatrix = parentModelMatrix *
                    socketTransform *
                    glm::scale(
                        g_translateMatrix * rotationMatrix,
                        scale);
            }
            else {
                m_modelMatrix = parentModelMatrix *
                    glm::scale(
                        g_translateMatrix * rotationMatrix,
                        scale);
            }
        }

        {
            // https://gamedev.stackexchange.com/questions/48927/how-can-you-extract-orientation-from-a-transformation-matrix
            m_modelScale.x = glm::length(m_modelMatrix[0]);
            m_modelScale.y = glm::length(m_modelMatrix[1]);
            m_modelScale.z = glm::length(m_modelMatrix[2]);

            assert(m_modelScale.x >= 0 && m_modelScale.y >= 0 && m_modelScale.z >= 0);

            m_modelRotation = parent.m_modelRotation * m_rotation * m_baseRotation;
        }

        {
            m_worldPivot = m_modelMatrix * glm::vec4{ m_pivotAlignment + m_pivotOffset, 1.f };
        }

        updateModelAxis();

        {
            m_parentMatrixLevel = parent.m_matrixLevel;
            m_matrixLevel++;
        }

        m_dirty = false;
        m_dirtySnapshot = true;
    }

    void NodeState::updateModelAxis() const noexcept
    {
        // NOTE KI "base quat" is assumed to have establish "normal" front dir
        // => thus no "base quat" here!
        // NOTE KI w == 0; only rotation
        m_viewFront = glm::normalize(glm::mat3(m_modelRotation * m_invBaseRotation) * m_front);
        glm::vec3 viewRight = glm::cross(m_viewFront, m_up);
        m_viewUp = glm::normalize(glm::cross(viewRight, m_viewFront));
    }
}
