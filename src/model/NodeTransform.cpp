#include "NodeTransform.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "util/thread.h"

#include "registry/EntitySSBO.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"

namespace {
    glm::mat4 shared_translateMatrix{ 1.f };
    glm::mat4 shared_scaleMatrix{ 1.f };
}

void NodeTransform::updateRootMatrix() noexcept
{
    ASSERT_WT();
    if (!m_dirty) return;

    updateRotationMatrix();

    shared_translateMatrix[3].x = m_position.x;
    shared_translateMatrix[3].y = m_position.y;
    shared_translateMatrix[3].z = m_position.z;

    shared_scaleMatrix[0].x = m_scale.x;
    shared_scaleMatrix[1].y = m_scale.y;
    shared_scaleMatrix[2].z = m_scale.z;

    m_modelMatrix = shared_translateMatrix * m_rotationMatrix * shared_scaleMatrix;
    m_modelScale = m_scale;

    {
        const auto& wp = m_modelMatrix[3];
        m_worldPos.x = wp.x;
        m_worldPos.y = wp.y;
        m_worldPos.z = wp.z;
    }

    updateModelAxis();

    m_dirty = false;
    m_matrixLevel++;
    m_dirtyEntity = true;
}

void NodeTransform::updateModelMatrix(const NodeTransform& parent) noexcept
{
    ASSERT_WT();
    if (!m_dirty && parent.m_matrixLevel == m_parentMatrixLevel) return;

    // NOTE KI only *SINGLE* thread is allowed to do model updates
    // => thus can use globally shared temp vars
    {
        shared_translateMatrix[3].x = m_position.x;
        shared_translateMatrix[3].y = m_position.y;
        shared_translateMatrix[3].z = m_position.z;

        shared_scaleMatrix[0].x = m_scale.x;
        shared_scaleMatrix[1].y = m_scale.y;
        shared_scaleMatrix[2].z = m_scale.z;
    }

    bool wasDirtyRotation = m_dirtyRotation;
    updateRotationMatrix();
    m_modelMatrix = parent.m_modelMatrix * shared_translateMatrix * m_rotationMatrix * shared_scaleMatrix;
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

    m_parentMatrixLevel = parent.m_matrixLevel;
    m_matrixLevel++;
    m_dirty = false;
    m_dirtyEntity = true;
    m_dirtySnapshot = true;
}

void NodeTransform::updateModelAxis() noexcept
{
    // NOTE KI "base quat" is assumed to have establish "normal" front dir
    // => thus no "base quad" here!
    // NOTE KI w == 0; only rotation
    m_viewFront = glm::normalize(glm::mat3(m_quatRotation) * m_front);

    m_viewRight = glm::normalize(glm::cross(m_viewFront, m_up));
    m_viewUp = glm::normalize(glm::cross(m_viewRight, m_viewFront));
}

void NodeTransform::updateRotationMatrix() noexcept
{
    ASSERT_WT();
    if (!m_dirtyRotation) return;
    m_rotationMatrix = glm::toMat4(m_quatRotation * m_baseRotation);
    m_dirtyRotation = false;
}

void NodeTransform::updateDegrees() const noexcept
{
    ASSERT_RT();
    if (!m_dirtyDegrees) return;
    m_degreesRotation = util::quatToDegrees(m_quatRotation);
    m_dirtyDegrees = false;
}

//void NodeTransform::updateEntity(
//    const UpdateContext& ctx,
//    EntitySSBO* entity)
//{
//    ASSERT_RT();
//    if (!m_dirtyEntity) return;
//
//    entity->u_materialIndex = m_materialIndex;
//    entity->u_shapeIndex = m_shapeIndex;
//
//    //entity->u_highlightIndex = getHighlightIndex(assets);
//
//    if (ctx.m_assets.frustumAny) {
//        m_volume.updateVolume(m_matrixLevel, m_modelMatrix, getWorldMaxScale());
//        entity->u_volume = m_volume.getWorldVolume();
//    }
//
//    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
//    entity->setModelMatrix(m_modelMatrix, m_uniformScale);
//
//    entity->u_worldScale = getWorldScale();
//
//    m_dirtyEntity = false;
//}
