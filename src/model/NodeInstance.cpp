#include "NodeInstance.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "registry/EntitySSBO.h"


void NodeInstance::setRotation(const glm::vec3& rotation) noexcept
{
    if (m_rotation != rotation) {
        m_rotation = rotation;
        m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
        m_dirty = true;
    }
}

void NodeInstance::updateEntity(
    EntitySSBO* entity)
{
    if (!m_entityDirty) return;

    entity->setObjectID(m_objectID);
    entity->u_flags = m_flags;
    entity->u_materialIndex = m_materialIndex;

    //entity->u_highlightIndex = getHighlightIndex(ctx);
    entity->u_volume = m_volume;

    entity->setModelMatrix(m_modelMatrix);
    // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
    entity->setNormalMatrix(glm::mat3(glm::inverseTranspose(m_modelMatrix)));

    m_entityDirty = false;
}
