#include "Light.h"

#include <string>
#include <cstdio>

#include "ki/sid.h"

#include "shader/LightsUBO.h"

#include "engine/UpdateContext.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"

#include "scene/Scene.h"
#include "scene/World.h"

Light::Light() = default;

Light::~Light() = default;

void Light::updateRT(
    const UpdateContext& ctx,
    const model::Node& node) noexcept
{
    if (!m_enabled) return;

    auto& nodeRegistry = *ctx.getRegistry()->m_nodeRegistry;

    const auto* snapshot = node.getSnapshotRT();
    if (!snapshot) return;

    const bool nodeChanged = m_nodeMatrixLevel != snapshot->getMatrixLevel();

    if (nodeChanged) {
        m_worldPosition = snapshot->getWorldPosition();
    }

    // Sun-driven directional light: derive direction + color from the scene World's
    // published (atomic) world time. RT-only — Light is RT-mutated and ShadowCascade
    // reads getWorldDirection(), so shadows follow automatically. World time advances
    // each frame, so this recomputes unconditionally (not gated on nodeChanged).
    if (m_sun) {
        if (auto* scene = ctx.getScene()) {
            if (auto* world = scene->getWorld().get()) {
                const double t = world->publishedTimeSecs();
                // active body (sun by day, moon by night) is always above the
                // horizon, so the light points downward -> no upward shadows
                const auto light = world->primaryLight(t);
                m_worldDir = light.dir;

                // chroma in diffuse, energy in intensity (kept separate):
                // intensity = base energy * horizon fade * (moon ? moon scale : 1)
                m_diffuse = light.color;
                m_intensity = m_baseIntensity * light.weight
                    * (light.isMoon ? m_moonIntensityScale : 1.f);
            }
        }
    }
    // NOTE KI for "directional" lights also target may change
    else if (hasTarget()) {
        if (!m_targetHandle) {
            m_targetHandle = pool::NodeHandle::toHandle(m_targetId);
        }
        auto* targetNode = m_targetHandle.toNode();

        if (!targetNode) {
            KI_WARN(fmt::format(
                "LIGHT: MISSING TARGET: {}-{}",
                m_targetId,
                SID_NAME(m_targetId)));

            targetNode = nodeRegistry.getRootRT();
        }

        if (!targetNode) return;

        const auto* targetSnapshot = targetNode->getSnapshotRT();
        if (!targetSnapshot) return;

        const bool targetChanged = m_targetMatrixLevel != targetSnapshot->getMatrixLevel();
        if (!(targetChanged || nodeChanged)) return;

        // worldTarget is relative to *ROOT*
        if (targetChanged) {
            m_worldTargetPosition = targetSnapshot->getWorldPosition();
        }

        // TODO KI SHOULD have local vs. world dir logic; or separate logic for "spot" light
        // => for spot light dir should be *NOT* calculated but set by initializer logic
        m_worldDir = glm::normalize(m_worldTargetPosition - m_worldPosition);

        m_targetMatrixLevel = targetSnapshot->getMatrixLevel();
    }

    {
        const float lightMax = std::fmaxf(std::fmaxf(m_diffuse.r, m_diffuse.g), m_diffuse.b);
        m_radius =
            (-m_linear + std::sqrtf(m_linear * m_linear - 4.f * m_quadratic *
                (m_constant - (256.f / 5.f) * lightMax))) / (2.f * m_quadratic);
    }

    m_nodeMatrixLevel = snapshot->getMatrixLevel();
}

DirLightUBO Light::toDirLightUBO() const noexcept
{
    return {
        m_worldDir,
        //0,

        glm::vec4(m_diffuse, m_intensity),
    };
}

PointLightUBO Light::toPointightUBO() const noexcept
{
    return {
        m_worldPosition,
        //0,

        glm::vec4(m_diffuse, m_intensity),

        m_constant,
        m_linear,
        m_quadratic,
        m_radius,
    };
}

SpotLightUBO Light::toSpotLightUBO() const noexcept
{
    return {
        m_worldPosition,
        //0,

        m_worldDir,
        //0,

        glm::vec4(m_diffuse, m_intensity),

        m_constant,
        m_linear,
        m_quadratic,

        m_cutoffAngle,
        m_outerCutoffAngle,
        m_radius,

        0,
        0,
    };
}
