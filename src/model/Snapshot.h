#pragma once

#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/size.h"
#include "ki/limits.h"

#include "backend/Lod.h"

#include "util/glm_util.h"

#include "pool/NodeHandle.h"


struct UpdateContext;

struct NodeState;
struct EntitySSBO;

//
// Snapshot of transform for RT
//
struct Snapshot {
private:
    glm::vec4 m_volume{ 0.f };

    //glm::vec3 m_worldPos{ 0.f };

    //// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    //glm::quat m_rotation{ 1.f, 0.f, 0.f, 0.f };

    glm::vec3 m_viewUp{ 0.f };
    glm::vec3 m_viewFront{ 0.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::vec3 m_modelScale{ 1.f };

public:
    //// NOTE KI static member fields rather safe to access directly
    //pool::NodeHandle m_handle;

    //uint32_t m_boneBaseIndex{ 0 };
    //uint32_t m_socketBaseIndex{ 0 };

    ki::size_t_entity_flags m_flags{ 0 }; // 1 * 4 = 4

    ki::level_id m_matrixLevel{ 0 };

    mutable bool m_dirty : 1 { true };
    mutable bool m_dirtyNormal : 1 { true };

public:
    ///////////////////////////////////////
    //
    Snapshot() = default;
    Snapshot(const NodeState& o);
    Snapshot(const NodeState&& o);

    void applyFrom(const NodeState& o) noexcept;

    //Snapshot& operator=(Snapshot& o) = default;
    inline void applyFrom(const Snapshot& o) noexcept
    {
        m_dirty |= o.m_dirty;
        m_dirtyNormal |= o.m_dirtyNormal;

        m_matrixLevel = o.m_matrixLevel;

        m_flags = o.m_flags;

        //m_boneBaseIndex = o.m_boneBaseIndex;
        //m_socketBaseIndex = o.m_socketBaseIndex;

        m_volume = o.m_volume;

        //m_worldPos = o.m_worldPos;

        //m_rotation = o.m_rotation;

        m_viewUp = o.m_viewUp;
        m_viewFront = o.m_viewFront;
        m_modelMatrix = o.m_modelMatrix;
        //m_handle = o.m_handle;

        m_modelScale = o.m_modelScale;

        o.m_dirty = false;
        o.m_dirtyNormal = false;
    }

    void setVolume(const glm::vec4& volume) noexcept
    {
        if (m_volume != volume) {
            m_dirty = true;
            m_volume = volume;
        }
    }

    inline const glm::vec4& getVolume() const noexcept
    {
        return m_volume;
    }

    //glm::vec3 getDegreesRotation() const noexcept;

    //inline const glm::quat& getRotation() const noexcept
    //{
    //    return m_rotation;
    //}

    inline const glm::vec3& getViewUp() const noexcept {
        return m_viewUp;
    }

    inline const glm::vec3& getViewFront() const noexcept {
        return m_viewFront;
    }

    inline glm::vec3 getViewRight() const noexcept {
        //return m_viewRight;
        return glm::cross(m_viewFront, m_viewUp);
    }

    inline glm::vec3 getWorldPosition() const noexcept
    {
        //return m_worldPos;
        return m_modelMatrix[3];
    }

    //inline const glm::vec3& getWorldScale() const noexcept
    //{
    //    return m_modelScale;
    //}

    //inline float getWorldMaxScale() const noexcept
    //{
    //    return std::max(std::max(m_modelScale.x, m_modelScale.y), m_modelScale.z);
    //}

    inline ki::level_id getMatrixLevel() const noexcept {
        return m_matrixLevel;
    }

    inline const glm::mat4& getModelMatrix() const noexcept {
        return m_modelMatrix;
    }

    void updateEntity(
        EntitySSBO& entity) const;
};
