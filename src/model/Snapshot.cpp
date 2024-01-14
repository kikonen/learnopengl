#include "Snapshot.h"

#include "util/thread.h"

#include "registry/EntitySSBO.h"

#include "model/NodeTransform.h"


namespace {
}

Snapshot::Snapshot(const NodeTransform& o)
    : m_matrixLevel{ o.m_matrixLevel },
    m_flags{ o.m_flags },
    m_materialIndex{ o.m_materialIndex },
    m_shapeIndex{ o.m_shapeIndex },
    m_volume{ o.m_volume.getVolume() },
    m_worldPos{ o.m_worldPos },
    m_quatRotation{ o.m_quatRotation },
    m_viewUp{ o.m_viewUp },
    m_viewFront{ o.m_viewFront },
    m_viewRight{ o.m_viewRight },
    m_modelMatrix{ o.m_modelMatrix },
    m_modelScale{ o.m_modelScale }
{
    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);
}

Snapshot::Snapshot(const NodeTransform&& o)
    : m_matrixLevel{ o.m_matrixLevel },
    m_flags{ o.m_flags },
    m_materialIndex{ o.m_materialIndex },
    m_shapeIndex{ o.m_shapeIndex },
    m_worldPos{ o.m_worldPos },
    m_quatRotation{ o.m_quatRotation },
    m_viewUp{ o.m_viewUp },
    m_viewFront{ o.m_viewFront },
    m_viewRight{ o.m_viewRight },
    m_modelMatrix{ o.m_modelMatrix },
    m_modelScale{ o.m_modelScale }
{
    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);
}

Snapshot& Snapshot::operator=(const NodeTransform& o) noexcept
{
    m_matrixLevel = o.m_matrixLevel;

    m_flags = o.m_flags;

    m_materialIndex = o.m_materialIndex;
    m_shapeIndex = o.m_shapeIndex;

    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);

    m_worldPos = o.m_worldPos;
    m_quatRotation = o.m_quatRotation;
    m_viewUp = o.m_viewUp;
    m_viewFront = o.m_viewFront;
    m_viewRight = o.m_viewRight;
    m_modelMatrix = o.m_modelMatrix;
    m_modelScale = o.m_modelScale;

    return *this;
}

Snapshot& Snapshot::operator=(Snapshot& o) noexcept
{
    m_dirty = o.m_dirty;
    m_matrixLevel = o.m_matrixLevel;

    m_flags = o.m_flags;

    m_materialIndex = o.m_materialIndex;
    m_shapeIndex = o.m_shapeIndex;

    m_volume = o.m_volume;

    m_worldPos = o.m_worldPos;
    m_quatRotation = o.m_quatRotation;
    m_viewUp = o.m_viewUp;
    m_viewFront = o.m_viewFront;
    m_viewRight = o.m_viewRight;
    m_modelMatrix = o.m_modelMatrix;
    m_modelScale = o.m_modelScale;

    return *this;
}

const glm::vec3& Snapshot::getDegreesRotation() const noexcept
{
    return util::quatToDegrees(m_quatRotation);
}

void Snapshot::updateEntity(
    EntitySSBO& entity)
{
    ASSERT_RT();

    entity.u_flags = m_flags;

    entity.u_materialIndex = m_materialIndex;
    entity.u_shapeIndex = m_shapeIndex;

    entity.u_volume = m_volume;

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    const auto uniformScale  = m_modelScale.x == m_modelScale.y && m_modelScale.x == m_modelScale.z;

    entity.setModelMatrix(m_modelMatrix, uniformScale, true);

    entity.u_worldScale = m_modelScale;
}
