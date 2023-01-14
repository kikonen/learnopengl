#pragma once

#include <glm/glm.hpp>

#include "ki/uuid.h"

#include "asset/Shader.h"
#include "asset/LightUBO.h"


class Node;
class RenderContext;


class Light final
{
public:
    Light();
    ~Light() = default;

    void update(const RenderContext& ctx, Node& node) noexcept;

    void markDirty() noexcept;

    const uuids::uuid& getTargetId() const noexcept;
    void setTargetId(const uuids::uuid& targetId) noexcept;

    const glm::vec3& getWorldPosition() const noexcept;
    const glm::vec3& getWorldTargetPosition() const noexcept;

    const glm::vec3& getPosition() const noexcept;
    void setPosition(const glm::vec3& pos) noexcept;

    DirLightUBO toDirLightUBO() const noexcept;
    PointLightUBO toPointightUBO() const noexcept;
    SpotLightUBO toSpotLightUBO() const noexcept;

public:
    bool m_enabled = true;
    bool m_directional = false;
    bool m_point = false;
    bool m_spot = false;

    // http://wiki.ogre3d.org/tiki-index.php?page=-Point+Light+Attenuation
    float constant = 1.f;
    float linear = 0.f;
    float quadratic = 0.f;

    // degrees
    float cutoffAngle = 0.f;
    // degrees
    float outerCutoffAngle = 0.f;

    float radius = 0.f;

    glm::vec4 ambient{ 0.2f, 0.2f, 0.2f, 1.f };
    glm::vec4 diffuse{ 0.5f, 0.5f, 0.5f, 1.f };
    glm::vec4 specular{ 1.0f, 1.0f, 1.0f, 1.f };

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
