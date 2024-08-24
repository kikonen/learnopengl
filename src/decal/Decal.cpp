#include "Decal.h"

#include <numbers>

#include "util/glm_util.h"
#include "util/Util.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "engine/UpdateContext.h"

namespace {
    constexpr float DECAL_DIST = 0.002f;
    inline glm::vec3 QUAD_NORMAL{ 0, 0, 1.f };
}

namespace decal {
    bool Decal::update(const UpdateContext& ctx) noexcept
    {
        const auto dt = ctx.m_clock.elapsedSecs;

        m_lifetime -= dt;
        if (m_lifetime <= 0) return false;

        if (m_spriteSpeed >= 0) {
            m_spriteActiveIndex += std::fmodf(dt * m_spriteSpeed, (float)m_spriteCount);
        }
        else {
            m_spriteActiveIndex += dt * m_spriteSpeed;
            if (m_spriteActiveIndex < 0) {
                m_spriteActiveIndex = ((float)m_spriteCount) + m_spriteActiveIndex;
            }
        }

        return true;
    }

    void Decal::updateSSBO(DecalSSBO& ssbo) const noexcept
    {
        ssbo.setTransform(getModelMatrix());

        ssbo.u_materialIndex = m_materialIndex;
        ssbo.u_spriteIndex = m_spriteBaseIndex +
            static_cast<uint8_t>(std::max(0.f, m_spriteActiveIndex));
    }

    // NOTE KI decal is rendered as Quad, so transform accordingly
    glm::mat4 Decal::getModelMatrix() const {
        const auto* node = m_parent.toNode();
        const auto& state = node->getState();

        const auto& parentMatrix = state.getModelMatrix();

        const auto& localTranslateMatrix = glm::translate(glm::mat4{ 1.f }, m_position);
        // local rotate around Z
        const auto& localRotationMatrix = glm::mat4(util::axisRadiansToQuat(glm::vec3{ 0.f, 0.f, 1.f }, m_rotation));

        // NOTE KI calculate rotation between Quad and parent node hit normal
        glm::mat4 rotationMatrix = glm::mat4{ util::normalToRotation(m_normal, QUAD_NORMAL) };

        return parentMatrix * localTranslateMatrix * rotationMatrix * localRotationMatrix;
    }

    Decal Decal::createForHit(
        const RenderContext& ctx,
        pool::NodeHandle parent,
        const glm::vec3& hitPos,
        const glm::vec3& hitNormal)
    {
        const auto* node = parent.toNode();
        if (!node) return {};

        const auto& snapshot = node->getActiveSnapshot(ctx);
        const auto invModelMatrix = glm::inverse(snapshot.getModelMatrix());

        decal::Decal decal{};
        decal.m_parent = parent;
        decal.m_position = invModelMatrix * glm::vec4(hitPos + hitNormal * DECAL_DIST, 1.f);
        decal.m_normal = glm::mat3(invModelMatrix) * hitNormal;

        decal.m_rotation = util::prnd(std::numbers::pi_v<float> / 2.f);

        float sign = util::prnd(10.f) > 5.f ? 1.f : -1.f;
        decal.m_spriteSpeed = sign * (5 + util::prnd(20.f));

        return decal;
    }
}
