#include "NodeInstance.h"

#include <glm/glm.hpp>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "asset/Sphere.h"

#include "registry/EntitySSBO.h"

#include "render/RenderContext.h"


void NodeInstance::updateRotationMatrix() noexcept
{
    if (!m_rotationDirty) return;
    m_rotationMatrix = glm::toMat4(glm::quat(glm::radians(m_rotation)));
    m_rotationDirty = false;
}

void NodeInstance::updateEntity(
    EntitySSBO* entity)
{
    if (!m_entityDirty) return;

    entity->setObjectID(m_objectID);
    entity->u_flags = m_flags;
    entity->u_materialIndex = m_materialIndex;

    //entity->u_highlightIndex = getHighlightIndex(assets);

    {
        m_volume.updateVolume(m_matrixLevel, m_modelMatrix, getWorldMaxScale());
        entity->u_volume = m_volume.getWorldVolume();
    }

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    bool uniformScale = isUniformScale();
    entity->setModelMatrix(m_modelMatrix, uniformScale);
    if (!uniformScale) {
        // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
        entity->setNormalMatrix(glm::mat3(glm::inverseTranspose(m_modelMatrix)));
    }

    entity->u_worldScale = getWorldScale();

    m_entityDirty = false;
}

//bool NodeInstance::inFrustum(const RenderContext& ctx, float radiusFlex) const
//{
//    //https://en.wikibooks.org/wiki/OpenGL_Programming/Glescraft_5
//    auto coords = ctx.m_matrices.u_projected * glm::vec4(getWorldPosition(), 1.0);
//    coords.x /= coords.w;
//    coords.y /= coords.w;
//
//    bool hit = true;
//    if (coords.x < -1 || coords.x > 1 || coords.y < -1 || coords.y > 1 || coords.z < 0) {
//        const auto& volume = getVolume();
//        float diameter = volume.a * radiusFlex;
//
//        if (coords.z < -diameter) {
//            hit = false;
//        }
//
//        if (hit) {
//            diameter /= fabsf(coords.w);
//            if (fabsf(coords.x) > 1 + diameter || fabsf(coords.y > 1 + diameter)) {
//                hit = false;
//            }
//        }
//    }
//    return hit;
//}
