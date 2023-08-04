#include "NodeInstance.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "registry/EntitySSBO.h"

#include "engine/UpdateContext.h"
#include "render/RenderContext.h"


void NodeInstance::updateRotationMatrix() noexcept
{
    if (!m_rotationDirty) return;
    m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
    m_rotationDirty = false;
}

void NodeInstance::updateEntity(
    const UpdateContext& ctx,
    EntitySSBO* entity)
{
    if (!m_entityDirty) return;

    entity->setObjectID(m_objectID);
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

    m_entityDirty = false;
}
