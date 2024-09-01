#include "NodeState.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "util/thread.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

namespace {
    const glm::vec3 ZERO_VEC{ 0.f };

    // NOTE KI only *SINGLE* thread is allowed to do model updates
    // => thus can use globally shared temp vars
    static glm::mat4 g_translateMatrix{ 1.f };
    static glm::mat4 g_scaleMatrix{ 1.f };
    static glm::mat4 g_pivotMatrix{ 1.f };
    static glm::mat4 g_invPivotMatrix{ 1.f };
}

glm::vec3 NodeState::getDegreesRotation() const noexcept
{
    return util::quatToDegrees(m_rotation);
}

void NodeState::updateRootMatrix() noexcept
{
    ASSERT_WT();
    // TODO KI why dirty track for root is not working?!?
    //if (!m_dirty) return;

    updateRotationMatrix();

    static glm::mat4 s_translateMatrix{ 1.f };
    static glm::mat4 s_scaleMatrix{ 1.f };
    {
        s_translateMatrix[3].x = m_position.x;
        s_translateMatrix[3].y = m_position.y;
        s_translateMatrix[3].z = m_position.z;

        s_scaleMatrix[0].x = m_scale.x;
        s_scaleMatrix[1].y = m_scale.y;
        s_scaleMatrix[2].z = m_scale.z;
    }

    m_modelMatrix = s_translateMatrix * m_rotationMatrix * s_scaleMatrix;
    m_modelScale = m_scale;

    {
        const auto& wp = m_modelMatrix[3];
        m_worldPos.x = wp.x;
        m_worldPos.y = wp.y;
        m_worldPos.z = wp.z;
    }

    m_matrixLevel++;

    m_dirty = false;
    m_dirtySnapshot = true;
}

void NodeState::updateModelMatrix(const NodeState& parent) noexcept
{
    ASSERT_WT();

    if (!m_dirty && parent.m_matrixLevel == m_parentMatrixLevel) return;
    {
        m_parentMatrixLevel = parent.m_matrixLevel;
        m_matrixLevel++;
    }

    const auto hasPivot = m_pivot != ZERO_VEC;

    {
        g_translateMatrix[3].x = m_position.x;
        g_translateMatrix[3].y = m_position.y;
        g_translateMatrix[3].z = m_position.z;

        g_scaleMatrix[0].x = m_scale.x;
        g_scaleMatrix[1].y = m_scale.y;
        g_scaleMatrix[2].z = m_scale.z;

        if (hasPivot) {
            g_pivotMatrix[3].x = -m_pivot.x * m_scale.x;
            g_pivotMatrix[3].y = -m_pivot.y * m_scale.y;
            g_pivotMatrix[3].z = -m_pivot.z * m_scale.z;

            g_invPivotMatrix[3].x = m_pivot.x * m_scale.x;
            g_invPivotMatrix[3].y = m_pivot.y * m_scale.y;
            g_invPivotMatrix[3].z = m_pivot.z * m_scale.z;
        }
    }

    updateRotationMatrix();

    if (hasPivot) {
        m_modelMatrix = parent.m_modelMatrix *
            g_translateMatrix *
            g_invPivotMatrix *
            m_rotationMatrix *
            g_pivotMatrix *
            g_scaleMatrix;
    }
    else {
        //m_modelMatrix = parent.m_modelMatrix *
        //    g_translateMatrix *
        //    m_rotationMatrix *
        //    g_scaleMatrix;

        m_modelMatrix = parent.m_modelMatrix *
            glm::scale(
                g_translateMatrix * m_rotationMatrix,
                m_scale);
    }


    m_modelScale = parent.m_modelScale * glm::mat3{ g_scaleMatrix };

    assert(m_modelScale.x >= 0 && m_modelScale.y >= 0 && m_modelScale.z >= 0);

    m_modelRotation = parent.m_modelRotation * m_rotation * m_baseRotation;

    {
        const auto& wp = m_modelMatrix[3];
        m_worldPos.x = wp.x;
        m_worldPos.y = wp.y;
        m_worldPos.z = wp.z;
    }

    {
        m_worldPivot = m_modelMatrix * glm::vec4{ m_pivot, 1.f };
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
    //m_viewRight = viewRight;

    m_dirtyAxis = false;
}

void NodeState::updateRotationMatrix() noexcept
{
    ASSERT_WT();
    if (!m_dirtyRotation) return;
    m_rotationMatrix = glm::toMat4(m_rotation * m_baseRotation);
    m_dirtyRotation = false;
    m_dirtyAxis = true;
}
