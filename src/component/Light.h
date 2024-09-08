#pragma once

#include <glm/glm.hpp>

#include "shader/LightUBO.h"

#include "ki/size.h"
#include "pool/NodeHandle.h"

struct UpdateContext;
class Node;


class Light final
{
public:
    Light() {};
    ~Light();

    void updateRT(const UpdateContext& ctx, Node& node) noexcept;

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
    bool m_enabled = false;
    bool m_directional = false;
    bool m_point = false;
    bool m_spot = false;

    // http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    float constant = 1.f;
    float linear = 0.14f;
    float quadratic = 0.07f;

    // degrees
    float cutoffAngle = 12.5f;
    // degrees
    float outerCutoffAngle = 25.f;

    float radius = 1.f;

    float intensity = 1.f;
    glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };

private:
    // dir = FROM pos to TARGET
    glm::vec3 m_worldDir{ 0.0f };
    glm::vec3 m_worldPosition{ 0.0f };
    glm::vec3 m_worldTargetPosition{ 0.0f };

    ki::node_id m_targetId;
    pool::NodeHandle m_targetHandle{};

    ki::level_id m_nodeMatrixLevel{ 0 };
    ki::level_id m_targetMatrixLevel{ 0 };
};
