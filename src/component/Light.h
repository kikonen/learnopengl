#pragma once

#include <glm/glm.hpp>

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

    const glm::vec3& getTargetPos() const noexcept;
    void setTargetPos(const glm::vec3& target) noexcept;

    const glm::vec3& getWorldPos() const noexcept;
    const glm::vec3& getWorldTargetPos() const noexcept;

    const glm::vec3& getPos() const noexcept;
    void setPos(const glm::vec3& pos) noexcept;

    DirLightUBO toDirLightUBO() const noexcept;
    PointLightUBO toPointightUBO() const noexcept;
    SpotLightUBO toSpotLightUBO() const noexcept;

public:
    bool enabled = true;
    bool directional = false;
    bool point = false;
    bool spot = false;

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
    bool m_dirty = true;

    // dir = FROM pos to TARGET
    glm::vec3 m_worldDir{ 0.0f };
    glm::vec3 m_worldPos{ 0.0f };
    glm::vec3 m_worldTargetPos{ 0.0f };

    // pos relative to owning node
    glm::vec3 m_pos{ 0.0f };

    // targetPos is relative to *root*
    glm::vec3 m_targetPos{ 0.f };

    int m_rootMatrixLevel = -1;
    int m_nodeMatrixLevel = -1;
};
