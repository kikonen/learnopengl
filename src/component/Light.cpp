#include "Light.h"

#include <string>
#include <cstdio>

#include "model/Node.h"
#include "scene/RenderContext.h"


Light::Light()
{
}

void Light::update(const RenderContext& ctx, Node& node)
{
    // NOTE KI not possible to truly know if some parent matrix has changed
    //if (!dirty) return;
    if (!enabled) return;

    m_worldPos = node.getWorldModelMatrixNoScale() * glm::vec4(m_pos, 1.f);
    // TODO KI SHOULD have local vs. world dir logic; or separate logic for "spot" light
    // => for spot light dir should be *NOT* calculated but set by initializer logic
    m_worldDir = glm::normalize(m_worldTarget - m_worldPos);

    if (!directional) {
        const float lightMax = std::fmaxf(std::fmaxf(diffuse.r, diffuse.g), diffuse.b);
        radius = (-linear + std::sqrtf(linear * linear - 4 * quadratic * (constant - (256.0 / 5.0) * lightMax))) / (2 * quadratic);
    }
}

const glm::vec3& Light::getPos()
{
    return m_pos;
}

void Light::setPos(const glm::vec3& pos)
{
    m_pos = pos;
    dirty = true;
}

const glm::vec3& Light::getWorldTarget()
{
    return m_worldTarget;
}

void Light::setWorldTarget(const glm::vec3& target)
{
    m_worldTarget = target;
    dirty = true;
}

const glm::vec3& Light::getWorldPos()
{
    return m_worldPos;
}

DirLightUBO Light::toDirLightUBO() {
    return { m_worldPos, enabled, m_worldDir, 0, ambient, diffuse, specular };
}

PointLightUBO Light::toPointightUBO()
{
    return {
        m_worldPos,
        enabled,

        ambient,
        diffuse,
        specular,

        constant,
        linear,
        quadratic,
        radius,
    };
}

SpotLightUBO Light::toSpotLightUBO()
{
    return {
        m_worldPos,
        enabled,
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
