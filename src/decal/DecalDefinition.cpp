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
        }

        decal.m_parent = parent;
        decal.m_position = invModelMatrix * glm::vec4(hitPos + hitNormal * DECAL_DIST, 1.f);
        decal.m_normal = glm::normalize(glm::mat3(invModelMatrix) * hitNormal);

        decal.m_rotation = util::prnd(std::numbers::pi_v<float> / 2.f);

        float sign = util::prnd(10.f) > 5.f ? 1.f : -1.f;
        decal.m_spriteSpeed = sign * (5 + util::prnd(20.f));

        return decal;
    }
}
