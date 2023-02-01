#include "Light.h"

#include <string>
#include <cstdio>

#include "asset/LightUBO.h"

#include "model/Node.h"
#include "scene/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


Light::Light()
{
}

void Light::update(const RenderContext& ctx, Node& node) noexcept
{
    if (!m_enabled) return;

    const bool nodeChanged = m_nodeMatrixLevel != node.getMatrixLevel();

    if (nodeChanged) {
        m_worldPosition = node.getModelMatrix() * glm::vec4(m_position, 1.f);
    }

    if (m_spot || m_directional) {
        Node* targetNode = ctx.m_registry->m_nodeRegistry->getNode(m_targetId);

        if (!targetNode) {
            KI_WARN(fmt::format("´LIGHT: MISSING TARGET: {}", KI_UUID_STR(m_targetId)));
            targetNode = ctx.m_registry->m_nodeRegistry->m_root;
        }

        if (!targetNode) return;

        const bool targetChanged = m_targetMatrixLevel != targetNode->getMatrixLevel();
        const bool changed = targetChanged || nodeChanged;
        if (!changed) return;

        // worldTarget is relative to *ROOT*
        if (targetChanged) {
            m_worldTargetPosition = targetNode->getWorldPosition();
        }

        // TODO KI SHOULD have local vs. world dir logic; or separate logic for "spot" light
        // => for spot light dir should be *NOT* calculated but set by initializer logic
        m_worldDir = glm::normalize(m_worldTargetPosition - m_worldPosition);

        m_targetMatrixLevel = targetNode->getMatrixLevel();
    }
    else {
        const bool changed = nodeChanged;
        if (!changed) return;
    }

    if (!m_directional) {
        const float lightMax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
        radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) / (2 * quadratic);
    }

    m_nodeMatrixLevel = node.getMatrixLevel();
}

void Light::markDirty() noexcept
{
    m_nodeMatrixLevel = -1;
    m_targetMatrixLevel = -1;
}

void Light::setPosition(const glm::vec3& pos) noexcept
{
    m_position = pos;
    m_nodeMatrixLevel = -1;
}

void Light::setTargetId(const uuids::uuid& targetId) noexcept
{
    m_targetId = targetId;
    m_targetMatrixLevel = -1;
}

DirLightUBO Light::toDirLightUBO() const noexcept
{
    return { m_worldPosition, 0, m_worldDir, 0, ambient, diffuse, specular };
}

PointLightUBO Light::toPointightUBO() const noexcept
{
    return {
        m_worldPosition,
        0,

        ambient,
        diffuse,
        specular,

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
        ambient,
        diffuse,
        specular,

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
