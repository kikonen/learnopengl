#pragma once

#include <glm/glm.hpp>

#include "shader/LightUBO.h"

#include "ki/size.h"
#include "pool/NodeHandle.h"

#include "LightType.h"

struct UpdateContext;
class Node;


class Light final
{
public:
    Light();
    ~Light();

    inline bool isDirectional() const noexcept
    {
        return m_type == LightType::directional;
    }

    inline bool isPoint() const noexcept
    {
        return m_type == LightType::point;
    }

    inline bool isSpot() const noexcept
    {
        return m_type == LightType::spot;
    }

    inline bool hasTarget() const noexcept
    {
        return m_type == LightType::directional || m_type == LightType::spot;
    }

    void updateRT(
        const UpdateContext& ctx,
        const Node& node) noexcept;

    void markDirty() noexcept
    {
        m_nodeMatrixLevel = -1;
        m_targetMatrixLevel = -1;
    }

    void setTargetId(ki::node_id targetId) noexcept
    {
        m_targetId = targetId;
        m_targetMatrixLevel = -1;
    }

    ki::node_id getTargetId() const noexcept {
        return m_targetId;
    }

    const glm::vec3& getWorldPosition() const noexcept {
        return m_worldPosition;
    }

    const glm::vec3& getWorldTargetPosition() const noexcept {
        return m_worldTargetPosition;
    }

    const glm::vec3& getWorldDirection() const noexcept {
        return m_worldDir;
    }

    DirLightUBO toDirLightUBO() const noexcept;
    PointLightUBO toPointightUBO() const noexcept;
    SpotLightUBO toSpotLightUBO() const noexcept;

public:
    glm::vec3 m_diffuse{ 0.5f, 0.5f, 0.5f };

    // http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    float m_constant = 1.f;
    float m_linear = 0.14f;
    float m_quadratic = 0.07f;

    // degrees
    float m_cutoffAngle = 12.5f;
    // degrees
    float m_outerCutoffAngle = 25.f;

    float m_intensity = 1.f;

    LightType m_type{ LightType::none };
    bool m_enabled : 1 { false };

private:
    ki::level_id m_nodeMatrixLevel{ 0 };
    ki::level_id m_targetMatrixLevel{ 0 };

    // dir = FROM pos to TARGET
    glm::vec3 m_worldDir{ 0.0f };
    glm::vec3 m_worldPosition{ 0.0f };
    glm::vec3 m_worldTargetPosition{ 0.0f };

    ki::node_id m_targetId;
    pool::NodeHandle m_targetHandle{};

    float m_radius = 1.f;
};
