#include "Snapshot.h"

#include "util/thread.h"

#include "model/EntityFlags.h"
#include "registry/EntitySSBO.h"

#include "model/NodeState.h"


namespace {
    constexpr ki::size_t_entity_flags PLANE_BITS = 0;
}

Snapshot::Snapshot(const NodeState& o)
    : m_matrixLevel{ o.m_matrixLevel },
    m_flags{ o.m_flags },
    m_tileIndex{ o.m_tileIndex },
    m_boneBaseIndex{ o.m_boneBaseIndex },
    m_socketBaseIndex{ o.m_socketBaseIndex },
    m_volume{ o.m_volume.getVolume() },
    m_worldPos{ o.m_worldPos },
    m_rotation{ o.m_rotation },
    m_viewUp{ o.m_viewUp },
    m_viewFront{ o.m_viewFront },
    //m_viewRight{ o.m_viewRight },
    m_modelMatrix{ o.m_modelMatrix },
    m_modelScale{ o.m_modelScale }
{
    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.m_worldPos, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);
}

Snapshot::Snapshot(const NodeState&& o)
    : m_matrixLevel{ o.m_matrixLevel },
    m_flags{ o.m_flags },
    m_tileIndex{ o.m_tileIndex },
    m_boneBaseIndex{ o.m_boneBaseIndex },
    m_socketBaseIndex{ o.m_socketBaseIndex },
    m_worldPos{ o.m_worldPos },
    m_rotation{ o.m_rotation },
    m_viewUp{ o.m_viewUp },
    m_viewFront{ o.m_viewFront },
    //m_viewRight{ o.m_viewRight },
    m_modelMatrix{ o.m_modelMatrix },
    m_modelScale{ o.m_modelScale }
{
    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.m_worldPos, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);
}

void Snapshot::applyFrom(const NodeState& o) noexcept
{
    m_matrixLevel = o.m_matrixLevel;

    m_dirty |= o.m_dirtySnapshot;
    m_dirtyNormal |= o.m_dirtyNormal;

    m_flags = o.m_flags;

    m_tileIndex = o.m_tileIndex;
    m_boneBaseIndex = o.m_boneBaseIndex;
    m_socketBaseIndex = o.m_socketBaseIndex;

    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.m_worldPos, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);

    m_worldPos = o.m_worldPos;

    m_rotation = o.m_rotation;

    m_viewUp = o.m_viewUp;
    m_viewFront = o.m_viewFront;
    //m_viewRight = o.m_viewRight;
    m_modelMatrix = o.m_modelMatrix;

    m_modelScale = o.m_modelScale;

    o.m_dirtySnapshot = false;
    o.m_dirtyNormal = false;
}

glm::vec3 Snapshot::getDegreesRotation() const noexcept
{
    return util::quatToDegrees(m_rotation);
}

void Snapshot::updateEntity(
    EntitySSBO& entity) const
{
    ASSERT_RT();

    entity.u_flags = m_flags;

    entity.u_volume = m_volume;

    entity.u_tileIndex = m_tileIndex;
    entity.u_boneBaseIndex = m_boneBaseIndex;
    entity.u_socketBaseIndex = m_socketBaseIndex;

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
