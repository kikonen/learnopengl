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

struct NodeTransform;
struct EntitySSBO;

//
// Snapshot of transform for RT
//
struct Snapshot {
    glm::vec4 m_volume{ 0.f };

    glm::vec3 m_worldPos{ 0.f };

    // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
    glm::quat m_quatRotation{ 1.f, 0.f, 0.f, 0.f };

    glm::vec3 m_viewUp{ 0.f };
    glm::vec3 m_viewFront{ 0.f };
    //glm::vec3 m_viewRight{ 0.f };

    glm::mat4 m_modelMatrix{ 1.f };
    glm::vec3 m_modelScale{ 1.f };

    pool::NodeHandle m_handle;

    uint32_t m_shapeIndex{ 0 };
    uint32_t m_boneIndex{ 0 };

    //std::array<int32_t, ki::MAX_LOD> m_lodMaterialIndeces;

    //// NOTE KI only in *active*
    //std::array<backend::Lod, ki::MAX_LOD> m_lods;

    ki::size_t_entity_flags m_flags{ 0 }; // 1 * 4 = 4

    ki::level_id m_matrixLevel{ 0 };

    mutable bool m_dirty : 1 { true };
    mutable bool m_dirtyNormal : 1 { true };

    ///////////////////////////////////////
    //
    Snapshot() = default;
    Snapshot(const NodeTransform& o);
    Snapshot(const NodeTransform&& o);

    void applyFrom(const NodeTransform& o) noexcept;

    //Snapshot& operator=(Snapshot& o) = default;
    inline void applyFrom(const Snapshot& o) noexcept
    {
        m_dirty |= o.m_dirty;
        m_dirtyNormal |= o.m_dirtyNormal;

        m_matrixLevel = o.m_matrixLevel;

        m_flags = o.m_flags;

        m_shapeIndex = o.m_shapeIndex;
        m_boneIndex = o.m_boneIndex;

        m_volume = o.m_volume;

        m_worldPos = o.m_worldPos;

        m_quatRotation = o.m_quatRotation;

        m_viewUp = o.m_viewUp;
        m_viewFront = o.m_viewFront;
        //m_viewRight = o.m_viewRight;
        m_modelMatrix = o.m_modelMatrix;
        m_handle = o.m_handle;

        m_modelScale = o.m_modelScale;

        //m_lodMaterialIndeces = o.m_lodMaterialIndeces;

        o.m_dirty = false;
        o.m_dirtyNormal = false;
    }

    inline const glm::vec4& getVolume() const noexcept
    {
        return m_volume;
    }

    glm::vec3 getDegreesRotation() const noexcept;

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

    inline glm::vec3 getViewRight() const noexcept {
        //return m_viewRight;
        return glm::cross(m_viewFront, m_viewUp);
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

    void updateEntity(
        EntitySSBO& entity) const;
};
