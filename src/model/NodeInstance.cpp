#include "NodeInstance.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "registry/EntitySSBO.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"


void NodeInstance::updateRootMatrix() noexcept
{
    if (!m_dirty) return;

    updateRotationMatrix();
    m_modelMatrix = m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
    m_modelScale = m_scaleMatrix;

    updateModelAxis();

    m_dirty = false;
    m_matrixLevel++;
    m_dirtyEntity = true;
}

void NodeInstance::updateModelMatrix(const NodeInstance& parent) noexcept
{
    if (!m_dirty && parent.m_matrixLevel == m_parentMatrixLevel) return;

    updateRotationMatrix();
    m_modelMatrix = parent.m_modelMatrix * m_translateMatrix * m_rotationMatrix * m_scaleMatrix;
    m_modelScale = parent.m_modelScale * m_scaleMatrix;

    updateModelAxis();

    m_dirty = false;
    m_parentMatrixLevel = parent.m_matrixLevel;
    m_matrixLevel++;
    m_dirtyEntity = true;
}

void NodeInstance::updateModelAxis() noexcept
{
    // NOTE KI "base quat" is assumed to have establish "normal" front dir
    // => thus no "base quad" here!
    // NOTE KI w == 0; only rotation
    m_viewFront = glm::normalize(glm::mat3(m_quatRotation) * m_front);

    m_viewRight = glm::normalize(glm::cross(m_viewFront, m_up));
    m_viewUp = glm::normalize(glm::cross(m_viewRight, m_viewFront));
}

void NodeInstance::updateRotationMatrix() noexcept
{
    if (!m_dirtyRotation) return;
    m_rotationMatrix = glm::toMat4(m_quatRotation * m_baseRotation);
    m_dirtyRotation = false;
}

void NodeInstance::updateEntity(
    const UpdateContext& ctx,
    EntitySSBO* entity)
{
    if (!m_dirtyEntity) return;

    entity->setId(m_id);
    entity->u_flags = m_flags;
    entity->u_materialIndex = m_materialIndex;
    entity->u_shapeIndex = m_shapeIndex;

    //entity->u_highlightIndex = getHighlightIndex(assets);

    if (ctx.m_assets.frustumAny) {
        m_volume.updateVolume(m_matrixLevel, m_modelMatrix, getWorldMaxScale());
        entity->u_volume = m_volume.getWorldVolume();
    }

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    entity->setModelMatrix(m_modelMatrix, m_uniformScale);
    if (!m_uniformScale) {
        // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
        // https://gamedev.stackexchange.com/questions/162248/correctly-transforming-normals-for-g-buffer-in-deferred-rendering
        // ???
        // "Then, for each scene object, compute their world space transforms,
        // and normal matrices. Tangent space (TBN) matrices can be computed
        // in the first pass shader.
        //
        // The normal matrix is the inverse transpose of the world space transform
        // (not object space to view space, as you would in a simpler forward rendering pipeline)."
        // ???
        entity->setNormalMatrix(glm::mat3(glm::inverseTranspose(m_modelMatrix)));
    }

    entity->u_worldScale = getWorldScale();

    m_dirtyEntity = false;
}
