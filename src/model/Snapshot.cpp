#include "Snapshot.h"

#include "util/thread.h"

#include "asset/Sphere.h"

#include "model/EntityFlags.h"
#include "registry/EntitySSBO.h"

#include "animation/AnimationSystem.h"

#include "model/NodeState.h"

namespace {
    constexpr ki::size_t_entity_flags PLANE_BITS = 0;

    Sphere s_sphere;
}

namespace model
{
    //Snapshot::Snapshot(const NodeState& o)
    //    : m_matrixLevel{ o.m_matrixLevel },
    //    m_flags{ o.m_flags },
    //    m_viewUp{ o.m_viewUp },
    //    m_viewFront{ o.m_viewFront },
    //    m_modelMatrix{ o.m_modelMatrix },
    //    m_modelScale{ o.m_modelScale },
    //    m_attachedSocketIndex{ o.m_attachedSocketIndex }
    //{
    //    glm::mat4 socketTransform{ 1.f };
    //    if (m_attachedSocketIndex) {
    //        socketTransform = animation::AnimationSystem::get().getSocketTransform(m_attachedSocketIndex);
    //    }
    //
    //    m_volume = Sphere::calculateWorldVolume(
    //        o.m_volume,
    //        o.m_modelMatrix * socketTransform,
    //        o.getWorldPosition(),
    //        o.getWorldMaxScale());
    //}

    //Snapshot::Snapshot(const NodeState&& o)
    //    : m_matrixLevel{ o.m_matrixLevel },
    //    m_flags{ o.m_flags },
    //    m_viewUp{ o.m_viewUp },
    //    m_viewFront{ o.m_viewFront },
    //    m_modelMatrix{ o.m_modelMatrix },
    //    m_modelScale{ o.m_modelScale },
    //    m_attachedSocketIndex{ o.m_attachedSocketIndex }
    //{
    //    glm::mat4 socketTransform{ 1.f };
    //    if (m_attachedSocketIndex) {
    //        socketTransform = animation::AnimationSystem::get().getSocketTransform(m_attachedSocketIndex);
    //    }
    //
    //    m_volume = Sphere::calculateWorldVolume(
    //        o.m_volume,
    //        o.m_modelMatrix * socketTransform,
    //        o.getWorldPosition(),
    //        o.getWorldMaxScale());
    //}

    void Snapshot::applyFrom(const NodeState& o) noexcept
    {
        m_matrixLevel = o.m_matrixLevel;

        m_dirty |= o.m_dirtySnapshot;
        m_dirtyNormal |= o.m_dirtyNormal;

        m_flags = o.m_flags;

        o.updateModelAxis();
        m_viewUp = o.m_viewUp;
        m_viewFront = o.m_viewFront;
        m_modelMatrix = o.m_modelMatrix;

        m_modelScale = o.m_modelScale;

        m_attachedSocketIndex = o.m_attachedSocketIndex;

        glm::mat4 socketTransform{ 1.f };
        if (m_attachedSocketIndex) {
            socketTransform = animation::AnimationSystem::get().getSocketTransform(m_attachedSocketIndex);
        }

        m_volume = Sphere::calculateWorldVolume(
            o.m_volume,
            o.m_modelMatrix * socketTransform,
            o.getWorldPosition(),
            o.getWorldMaxScale());

        o.m_dirtySnapshot = false;
        o.m_dirtyNormal = false;
    }

    void Snapshot::updateEntity(
        EntitySSBO& entity) const
    {
        ASSERT_RT();

        entity.u_flags = m_flags;

        entity.u_volume = m_volume;

        // NOTE KI M-T matrix needed *ONLY* if non uniform scale
        // NOTE KI flat planes are *always* uniform, since problem with normal scaling does
        // not truly affect them
        const auto uniformScale = (m_flags & PLANE_BITS) != 0
            || (m_modelScale.x == m_modelScale.y && m_modelScale.x == m_modelScale.z);

        // NOTE KI normal may change only if scale changes
        entity.setModelMatrix(m_modelMatrix, uniformScale, m_dirtyNormal);

        entity.u_worldScale = m_modelScale;

        m_dirtyNormal = false;
    }
}
