#include "Decal.h"

#include <numbers>

#include "util/debug.h"
#include "util/glm_util.h"
#include "util/glm_format.h"
#include "util/util.h"
#include "util/log.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "engine/UpdateContext.h"

namespace {
    inline glm::vec3 QUAD_NORMAL{ 0.f, 0.f, 1.f };
    inline glm::vec3 QUAD_TANGENT{ 1.f, 0.f, 0.f };
    inline glm::vec3 UP{ 0, 1.f, 0 };
    inline glm::mat4 ID_MAT{ 1.f };
}

namespace decal {
    bool Decal::update(const UpdateContext& ctx) noexcept
    {
        const auto dt = ctx.getClock().elapsedSecs;

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

        const auto& localTranslateMatrix = glm::translate(glm::mat4{ 1.f }, m_position);

        // local rotate around QUAD_NORMAL
        const auto& localRotation = util::axisRadiansToQuat(QUAD_NORMAL, m_rotation);

        // NOTE KI calculate rotation between Quad and parent node hit normal
        const auto& rotation = glm::rotation(QUAD_NORMAL, m_normal);

        const glm::mat4 rotationMatrix = glm::mat4{ rotation * localRotation };

        // NOTE KI inverse scale so that parent's scale won't affect decal size
        // (only model scale, no world scale)
        const glm::mat4 invParentcaleMatrix = glm::scale(ID_MAT, 1.f / scale);
        const glm::mat4 scaleMatrix = glm::scale(ID_MAT, glm::vec3{ m_scale });

        const auto& mat = parentMatrix * localTranslateMatrix * invParentcaleMatrix * rotationMatrix * scaleMatrix;

        //KI_INFO_OUT(fmt::format(
        //    "DECAL: pos={}, normal={}, scale={}, parentScale={}, parent={}, mat={}",
        //    m_position, m_normal, m_scale, scale, parentMatrix, mat));

        return mat;
    }
}
