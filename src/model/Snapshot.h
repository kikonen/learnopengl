#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/size.h"

#include "util/glm_util.h"


class UpdateContext;

struct NodeTransform;
struct EntitySSBO;

//
// Snapshot of transform for RT 
//
struct Snapshot {
    Snapshot() = default;
    Snapshot(const NodeTransform& o);
    Snapshot(const NodeTransform&& o);

    Snapshot& operator=(const NodeTransform& o) noexcept;

    mutable bool m_dirtyDegrees{ true };
    bool m_dirtyNormal{ true };
    bool m_dirtyEntity{ true };
    bool m_uniformScale { false };

    ki::level_id m_matrixLevel{ (ki::level_id)-1 };

    int m_entityIndex{ -1 };

    int m_materialIndex{ 0 };
    int m_shapeIndex{ 0 };

    glm::vec4 m_volume{ 0.f };

    glm::vec3 m_worldPos{ 0.f };

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    glm::quat m_quatRotation{ 1.f, 0.f, 0.f, 0.f };
    mutable glm::vec3 m_degreesRotation{ 0.f };

    glm::vec3 m_viewUp{ 0.f };
    glm::vec3 m_viewFront{ 0.f };
    glm::vec3 m_viewRight{ 0.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::vec3 m_modelScale{ 1.f };


    inline const glm::vec4& getVolume() const noexcept
    {
        return m_volume;
    }

    inline const bool isUniformScale() const noexcept
    {
        return m_uniformScale;
    }

    inline const glm::vec3& getDegreesRotation() const noexcept
    {
        updateDegrees();
        return m_degreesRotation;
    }

    inline const glm::quat& getQuatRotation() const noexcept
    {
        return m_quatRotation;
    }

    inline const glm::vec3& getViewUp() const noexcept {
        return m_viewUp;
    }

    inline const glm::vec3& getViewFront() const noexcept {
        return m_viewFront;
    }

    inline const glm::vec3& getViewRight() const noexcept {
        return m_viewRight;
    }

    inline const glm::vec3& getWorldPosition() const noexcept
    {
        return m_worldPos;
    }

    inline const glm::vec3& getWorldScale() const noexcept
    {
        return m_modelScale;
    }

    inline float getWorldMaxScale() const noexcept
    {
        return std::max(std::max(m_modelScale.x, m_modelScale.y), m_modelScale.z);
    }

    inline ki::level_id getMatrixLevel() const noexcept {
        return m_matrixLevel;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_modelMatrix;
    }

    void updateDegrees() const noexcept;

    void updateEntity(
        const UpdateContext& ctx,
        EntitySSBO* entity);
};
