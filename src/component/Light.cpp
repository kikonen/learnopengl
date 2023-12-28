#include "Light.h"

#include <string>
#include <cstdio>

#include "asset/LightUBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


void Light::update(const UpdateContext& ctx, Node& node) noexcept
{
    if (!m_enabled) return;

    const bool nodeChanged = m_nodeMatrixLevel != node.getTransform().getMatrixLevel();

    if (nodeChanged) {
        m_worldPosition = node.getTransform().getWorldPosition();
    }

    if (m_spot || m_directional) {
        if (!m_targetNode) {
            m_targetNode = ctx.m_registry->m_nodeRegistry->getNode(m_targetId);
        }
        auto* targetNode = m_targetNode;

        if (!targetNode) {
            KI_WARN(fmt::format("´LIGHT: MISSING TARGET: {}", KI_UUID_STR(m_targetId)));
            targetNode = ctx.m_registry->m_nodeRegistry->m_root;
        }

        if (!targetNode) return;

        const bool targetChanged = m_targetMatrixLevel != targetNode->getTransform().getMatrixLevel();
        const bool changed = targetChanged || nodeChanged;
        if (!changed) return;

        // worldTarget is relative to *ROOT*
        if (targetChanged) {
            m_worldTargetPosition = targetNode->getTransform().getWorldPosition();
        }

        // TODO KI SHOULD have local vs. world dir logic; or separate logic for "spot" light
        // => for spot light dir should be *NOT* calculated but set by initializer logic
        m_worldDir = glm::normalize(m_worldTargetPosition - m_worldPosition);

        m_targetMatrixLevel = targetNode->getTransform().getMatrixLevel();
    }
    else {
        const bool changed = nodeChanged;
        if (!changed) return;
    }

    if (!m_directional) {
        const float lightMax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
        radius = (-linear + std::sqrtf(linear * linear - 4.f * quadratic * (constant - (256.f / 5.f) * lightMax))) / (2.f * quadratic);
    }

    m_nodeMatrixLevel = node.getTransform().getMatrixLevel();
}

DirLightUBO Light::toDirLightUBO() const noexcept
{
    return {
        m_worldDir,
        0,

        glm::vec4(diffuse, intensity),
    };
}

PointLightUBO Light::toPointightUBO() const noexcept
{
    return {
        m_worldPosition,
        0,

        glm::vec4(diffuse, intensity),

        constant,
        linear,
        quadratic,
        radius,
    };
}

SpotLightUBO Light::toSpotLightUBO() const noexcept
{
    return {
        m_worldPosition,
        0,

        m_worldDir,
        0,

        glm::vec4(diffuse, intensity),

        constant,
        linear,
        quadratic,

        cutoffAngle,
        outerCutoffAngle,
        radius,

        0,
        0,
    };
}
