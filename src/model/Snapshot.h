#pragma once

#include <array>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/SphereVolume.h"

#include "ki/size.h"
#include "ki/limits.h"

#include "backend/Lod.h"

#include "util/glm_util.h"

#include "pool/NodeHandle.h"

namespace model
{
    struct NodeState;
}

struct UpdateContext;

struct EntitySSBO;

namespace model
{
    //
    // Snapshot of transform for RT
    //
    struct Snapshot {
    private:
        SphereVolume m_worldVolume{ 0.f };

        glm::vec3 m_viewUp{ 0.f };
        glm::vec3 m_viewFront{ 0.f };

        glm::mat4 m_modelMatrix{ 1.f };
        glm::vec3 m_modelScale{ 1.f };

        // parent socket
        uint32_t m_attachedSocketIndex{ 0 };

    public:
        ki::size_t_entity_flags m_flags{ 0 }; // 1 * 4 = 4

        ki::level_id m_matrixLevel{ 0 };
        ki::level_id m_normalLevel{ 0 };

    public:
        ///////////////////////////////////////
        //
        Snapshot() = default;
        //Snapshot(const NodeState& o);
        //Snapshot(const NodeState&& o) noexcept;

        void applyFrom(const NodeState& o) noexcept;

        //Snapshot& operator=(Snapshot& o) = default;
        inline void applyFrom(const Snapshot& o) noexcept
        {
            m_matrixLevel = o.m_matrixLevel;
            m_normalLevel = o.m_normalLevel;

            m_flags = o.m_flags;

            m_worldVolume = o.m_worldVolume;

            m_viewUp = o.m_viewUp;
            m_viewFront = o.m_viewFront;
            m_modelMatrix = o.m_modelMatrix;

            m_modelScale = o.m_modelScale;

            m_attachedSocketIndex = o.m_attachedSocketIndex;
        }

        inline float getMaxScale() const noexcept
        {
            return std::max(m_modelScale.x, std::max(m_modelScale.y, m_modelScale.z));
        }

        inline const SphereVolume& getWorldVolume() const noexcept
        {
            return m_worldVolume;
        }

        inline const void setWorldVolume(const SphereVolume& worldVolume) noexcept
        {
            m_worldVolume = worldVolume;
        }

        inline const glm::vec3& getViewUp() const noexcept {
            return m_viewUp;
        }

        inline const glm::vec3& getViewFront() const noexcept {
            return m_viewFront;
        }

        inline glm::vec3 getViewRight() const noexcept {
            return glm::cross(m_viewFront, m_viewUp);
        }

        inline glm::vec3 getWorldPosition() const noexcept
        {
            return m_modelMatrix[3];
        }

        inline ki::level_id getMatrixLevel() const noexcept {
            return m_matrixLevel;
        }

        inline const glm::mat4& getModelMatrix() const noexcept {
            return m_modelMatrix;
        }

        void updateEntity(
            EntitySSBO& entity,
            bool dirtyNormal) const;
    };
}
