#include "Light.h"

#include <string>
#include <cstdio>

#include "asset/LightUBO.h"

#include "model/Node.h"
#include "scene/RenderContext.h"
#include "registry/NodeRegistry.h"


Light::Light()
{
}

void Light::update(const RenderContext& ctx, Node& node) noexcept
{
    if (!enabled) return;

    bool worldChanged = m_rootMatrixLevel != ctx.m_nodeRegistry.m_root->getMatrixLevel();
    bool nodeChanged = m_nodeMatrixLevel != node.getMatrixLevel();
    bool changed = m_dirty || worldChanged || nodeChanged;

    if (!changed) return;

    // worldTarget is relative to *ROOT*
    if (m_dirty || worldChanged) {
        m_worldTargetPos = ctx.m_nodeRegistry.m_root->getModelMatrix() * glm::vec4(m_targetPos, 1.0);
    }

    if (m_dirty || nodeChanged) {
        m_worldPos = node.getModelMatrix() * glm::vec4(m_pos, 1.f);
    }

    // TODO KI SHOULD have local vs. world dir logic; or separate logic for "spot" light
    // => for spot light dir should be *NOT* calculated but set by initializer logic
    m_worldDir = glm::normalize(m_worldTargetPos - m_worldPos);

    if (!directional) {
        const float lightMax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
        radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) / (2 * quadratic);
    }

    m_rootMatrixLevel = ctx.m_nodeRegistry.m_root->getMatrixLevel();
    m_nodeMatrixLevel = node.getMatrixLevel();
    m_dirty = false;
}

void Light::markDirty() noexcept
{
    m_dirty = true;
}

const glm::vec3& Light::getPos() const noexcept
{
    return m_pos;
}

void Light::setPos(const glm::vec3& pos) noexcept
{
    m_pos = pos;
    m_dirty = true;
}

const glm::vec3& Light::getTargetPos() const noexcept
{
    return m_targetPos;
}

void Light::setTargetPos(const glm::vec3& target) noexcept
{
    m_targetPos = target;
    m_dirty = true;
}

const glm::vec3& Light::getWorldPos() const noexcept
{
    return m_worldPos;
}

const glm::vec3& Light::getWorldTargetPos() const noexcept
{
    return m_worldTargetPos;
}

DirLightUBO Light::toDirLightUBO() const noexcept
{
    return { m_worldPos, 0, m_worldDir, 0, ambient, diffuse, specular };
}

PointLightUBO Light::toPointightUBO() const noexcept
{
    return {
        m_worldPos,
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
        m_worldPos,
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
