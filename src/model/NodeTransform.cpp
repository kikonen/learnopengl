#include "NodeTransform.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "util/thread.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

namespace {
}

void NodeTransform::updateRootMatrix() noexcept
{
    ASSERT_WT();
    if (!m_dirty) return;

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

    updateModelAxis();

    m_matrixLevel++;

    m_dirty = false;
    m_dirtySnapshot = true;
}

void NodeTransform::updateModelMatrix(const NodeTransform& parent) noexcept
{
    ASSERT_WT();

    if (!m_dirty && parent.m_matrixLevel == m_parentMatrixLevel) return;
    {
        m_parentMatrixLevel = parent.m_matrixLevel;
        m_matrixLevel++;
    }

    // NOTE KI only *SINGLE* thread is allowed to do model updates
    // => thus can use globally shared temp vars
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

    bool wasDirtyRotation = m_dirtyRotation;
    updateRotationMatrix();
    m_modelMatrix = parent.m_modelMatrix * s_translateMatrix * m_rotationMatrix * s_scaleMatrix;
    m_modelScale = parent.m_modelScale * m_scale;

    {
        const auto& wp = m_modelMatrix[3];
        m_worldPos.x = wp.x;
        m_worldPos.y = wp.y;
        m_worldPos.z = wp.z;
    }

    if (wasDirtyRotation) {
        updateModelAxis();
    }

    m_dirty = false;
    m_dirtySnapshot = true;
}

void NodeTransform::updateModelAxis() noexcept
{
    // NOTE KI "base quat" is assumed to have establish "normal" front dir
    // => thus no "base quad" here!
    // NOTE KI w == 0; only rotation
    m_viewFront = glm::mat3(m_quatRotation) * m_front;

    glm::vec3 viewRight = glm::cross(m_viewFront, m_up);
    m_viewUp = glm::cross(viewRight, m_viewFront);
}

void NodeTransform::updateRotationMatrix() noexcept
{
    ASSERT_WT();
    if (!m_dirtyRotation) return;
    m_rotationMatrix = glm::toMat4(m_quatRotation * m_baseRotation);
    m_dirtyRotation = false;
}
