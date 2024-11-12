#include "DecalDefinition.h"

#include <numbers>

#include "util/debug.h"
#include "util/glm_util.h"
#include "util/util.h"

#include "model/Node.h"
#include "model/Snapshot.h"

namespace {
    constexpr float DECAL_DIST = 0.003f;
}

namespace decal {
    Decal DecalDefinition::createForHit(
        const RenderContext& ctx,
        pool::NodeHandle parent,
        const glm::vec3& hitPos,
        const glm::vec3& hitNormal)
    {
        const auto* node = parent.toNode();
        if (!node) return {};

        const auto* snapshot = node->getSnapshotRT();
        if (!snapshot) return {};

        const auto invModelMatrix = glm::inverse(snapshot->getModelMatrix());

        decal::Decal decal{};
        {
            decal.m_materialIndex = m_materialIndex;
            decal.m_lifetime = m_lifetime;
            decal.m_scale = m_scale;
            decal.m_rotation = m_rotation;
            decal.m_spriteBaseIndex = m_spriteBaseIndex;
            decal.m_spriteCount = m_spriteCount;

            const auto rotationRange = m_rotationVariation.y - m_rotationVariation.x;
            decal.m_rotation = glm::radians(m_rotation + m_rotationVariation.x + util::prnd(1.f) * rotationRange);

            const auto scaleRange = m_scaleVariation.y - m_scaleVariation.x;
            decal.m_scale = m_scale  + m_scaleVariation.x + util::prnd(1.f) * scaleRange;

            const auto speedRange = m_spriteSpeedVariation.y - m_spriteSpeedVariation.x;
            decal.m_spriteSpeed = m_spriteSpeed + m_spriteSpeedVariation.x + util::prnd(1.f) * speedRange;
        }

        decal.m_parent = parent;
        decal.m_position = invModelMatrix * glm::vec4(hitPos + hitNormal * DECAL_DIST, 1.f);
        decal.m_normal = glm::normalize(glm::mat3(invModelMatrix) * hitNormal);

        return decal;
    }
}
