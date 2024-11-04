#include "Decal.h"

#include <numbers>

#include "util/debug.h"
#include "util/glm_util.h"
#include "util/util.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "engine/UpdateContext.h"

namespace {
    inline glm::vec3 QUAD_NORMAL{ 0, 0, 1.f };
    inline glm::mat4 ID_MAT{ 1.f };
}

namespace decal {
    bool Decal::update(const UpdateContext& ctx) noexcept
    {
        const auto dt = ctx.m_clock.elapsedSecs;

        m_lifetime -= dt;
        if (m_lifetime <= 0) return false;

        if (m_spriteSpeed >= 0) {
            m_spriteActiveIndex = std::fmodf(m_spriteActiveIndex + dt * m_spriteSpeed, (float)m_spriteCount);
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
        const auto& scale = state.getScale();

        //KI_INFO_OUT(fmt::format(
        //    "DECAL: pos={}, normal={}, scale={}, parentScale={}, parent={}",
        //    m_position, m_normal, m_scale, scale, parentMatrix));

        const auto& localTranslateMatrix = glm::translate(glm::mat4{ 1.f }, m_position);
        // local rotate around QUAD_NORMAL
        const auto& localRotationMatrix = glm::mat4(util::axisRadiansToQuat(QUAD_NORMAL, m_rotation));

        // NOTE KI calculate rotation between Quad and parent node hit normal
        glm::mat4 rotationMatrix = glm::mat4{ util::normalToRotation(m_normal, QUAD_NORMAL) };
        // NOTE KI inverse scale so that parent's scale won't affect decal size
        // (only model scale, no world scale)
        glm::mat4 invScaleMatrix = glm::scale(ID_MAT, 1.f / scale);
        glm::mat4 scaleMatrix = glm::scale(ID_MAT, glm::vec3{ m_scale });

        return parentMatrix * localTranslateMatrix * invScaleMatrix * rotationMatrix * localRotationMatrix * scaleMatrix;
    }
}
