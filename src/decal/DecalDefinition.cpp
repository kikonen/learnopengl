#include "DecalDefinition.h"

#include <numbers>

#include "util/debug.h"
#include "util/glm_util.h"
#include "util/util.h"

#include "model/Node.h"
#include "model/Snapshot.h"

namespace {
    constexpr float DECAL_DIST = 0.003f;

    float variant(float base, glm::vec2 variation)
    {
        const auto range = variation.y - variation.x;
        return base + variation.x + util::prnd(1.f) * range;
    }
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
            decal.m_scale = m_scale;
            decal.m_rotation = m_rotation;
            decal.m_spriteBaseIndex = m_spriteBaseIndex;
            decal.m_spriteCount = m_spriteCount;

            decal.m_lifetime = variant(m_lifetime, m_lifetimeVariation);
            decal.m_rotation = glm::radians(variant(m_rotation, m_rotationVariation));
            decal.m_scale = variant(m_scale, m_scaleVariation);
            decal.m_spriteSpeed = variant(m_spriteSpeed, m_spriteSpeedVariation);
        }

        decal.m_parent = parent;
        decal.m_position = invModelMatrix * glm::vec4(hitPos + hitNormal * DECAL_DIST, 1.f);
        decal.m_normal = glm::normalize(glm::mat3(invModelMatrix) * hitNormal);

        return decal;
    }
}
