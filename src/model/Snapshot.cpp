#include "Snapshot.h"

#include "util/thread.h"

#include "asset/Sphere.h"

#include "model/EntityFlags.h"
#include "registry/EntitySSBO.h"

#include "model/NodeState.h"

namespace {
    constexpr ki::size_t_entity_flags PLANE_BITS = 0;

    Sphere s_sphere;
}

namespace model
{
    void Snapshot::applyFrom(const NodeState& o) noexcept
    {
        m_matrixLevel = o.m_matrixLevel;
        m_normalLevel = o.m_normalLevel;

        m_flags = o.m_flags;

        //o.updateModelAxis();
        m_viewUp = o.m_viewUp;
        m_viewFront = o.m_viewFront;
        m_modelMatrix = o.m_modelMatrix;

        m_modelScale = o.m_modelScale;

        m_attachedSocketIndex = o.m_attachedSocketIndex;

        m_worldVolume = Sphere::calculateWorldVolume(
            o.m_localVolume,
            o.m_modelMatrix,
            o.getWorldPosition(),
            o.getWorldMaxScale());

        o.m_dirtySnapshot = false;
    }

    void Snapshot::updateEntity(
        EntitySSBO& entity,
        bool dirtyNormal) const
    {
        ASSERT_RT();

        entity.u_flags = m_flags;

        entity.u_worldVolume = m_worldVolume.toVec4();

        // NOTE KI M-T matrix needed *ONLY* if non uniform scale
        // NOTE KI flat planes are *always* uniform, since problem with normal scaling does
        // not truly affect them
        const auto uniformScale = (m_flags & PLANE_BITS) != 0
            || (m_modelScale.x == m_modelScale.y && m_modelScale.x == m_modelScale.z);

        // NOTE KI normal may change only if scale changes
        entity.setModelMatrix(m_modelMatrix, uniformScale, dirtyNormal);

        entity.u_worldScale = m_modelScale;
    }
}
