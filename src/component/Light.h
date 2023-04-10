#pragma once

#include <glm/glm.hpp>

#include "ki/uuid.h"

#include "asset/LightUBO.h"


class Node;
class UpdateContext;


class Light final
{
public:
    Light() {};
    ~Light() = default;

    void update(const UpdateContext& ctx, Node& node) noexcept;

    void markDirty() noexcept
    {
        m_nodeMatrixLevel = -1;
        m_targetMatrixLevel = -1;
    }

    void setTargetId(const uuids::uuid& targetId) noexcept
    {
        m_targetId = targetId;
        m_targetMatrixLevel = -1;
    }

    const uuids::uuid& getTargetId() const noexcept {
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

    const glm::vec3& getPosition() const noexcept {
        return m_position;
    }

    void setPosition(const glm::vec3& pos) noexcept
    {
        m_position = pos;
        m_nodeMatrixLevel = -1;
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

    glm::vec4 ambient{ 1.f, 1.f, 1.f, 1.f };
    glm::vec4 diffuse{ 0.7f, 0.7f, 0.7f, 1.f };
    glm::vec4 specular{ 0.6f, 0.6f, 0.6f, 1.f };

private:
    // dir = FROM pos to TARGET
    glm::vec3 m_worldDir{ 0.0f };
    glm::vec3 m_worldPosition{ 0.0f };
    glm::vec3 m_worldTargetPosition{ 0.0f };

    // pos relative to owning node
    glm::vec3 m_position{ 0.0f };

    uuids::uuid m_targetId;

    int m_nodeMatrixLevel = -1;
    int m_targetMatrixLevel = -1;
};
