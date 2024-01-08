#include "Snapshot.h"

#include "util/thread.h"

#include "registry/EntitySSBO.h"

#include "model/NodeTransform.h"


namespace {
}

Snapshot::Snapshot(const NodeTransform& o)
    : m_dirtyDegrees{ o.m_dirtyDegrees },
    m_dirtyNormal{ o.m_dirtyNormal },
    m_dirtyEntity{ o.m_dirtyEntity },
    m_uniformScale{ o.m_uniformScale },
    m_matrixLevel{ o.m_matrixLevel },
    m_entityIndex{ o.m_entityIndex },
    m_materialIndex{ o.m_materialIndex },
    m_shapeIndex{ o.m_shapeIndex },
    m_volume{ o.m_volume.getVolume() },
    m_worldPos{ o.m_worldPos },
    m_quatRotation{ o.m_quatRotation },
    m_degreesRotation{ o.m_degreesRotation },
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
    : m_dirtyDegrees{ o.m_dirtyDegrees },
    m_dirtyNormal{ o.m_dirtyNormal },
    m_dirtyEntity{ o.m_dirtyEntity },
    m_uniformScale{ o.m_uniformScale },
    m_matrixLevel{ o.m_matrixLevel },
    m_entityIndex{ o.m_entityIndex },
    m_materialIndex{ o.m_materialIndex },
    m_shapeIndex{ o.m_shapeIndex },
    m_worldPos{ o.m_worldPos },
    m_quatRotation{ o.m_quatRotation },
    m_degreesRotation{ o.m_degreesRotation },
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
    m_dirtyDegrees = o.m_dirtyDegrees;
    m_dirtyNormal = o.m_dirtyNormal;
    m_dirtyEntity = o.m_dirtyEntity;
    m_uniformScale = o.m_uniformScale;
    m_matrixLevel = o.m_matrixLevel;
    m_entityIndex = o.m_entityIndex;
    m_materialIndex = o.m_materialIndex;
    m_shapeIndex = o.m_shapeIndex;

    o.m_volume.updateVolume(o.m_matrixLevel, o.m_modelMatrix, o.getWorldMaxScale());
    o.m_volume.storeWorldVolume(m_volume);

    m_worldPos = o.m_worldPos;
    m_quatRotation = o.m_quatRotation;
    m_degreesRotation = o.m_degreesRotation;
    m_viewUp = o.m_viewUp;
    m_viewFront = o.m_viewFront;
    m_viewRight = o.m_viewRight;
    m_modelMatrix = o.m_modelMatrix;
    m_modelScale = o.m_modelScale;

    return *this;
}

Snapshot& Snapshot::operator=(Snapshot& o) noexcept
{
    m_dirtyDegrees = o.m_dirtyDegrees;
    m_dirtyNormal = o.m_dirtyNormal;
    m_dirtyEntity = o.m_dirtyEntity;
    m_uniformScale = o.m_uniformScale;
    m_matrixLevel = o.m_matrixLevel;
    m_entityIndex = o.m_entityIndex;
    m_materialIndex = o.m_materialIndex;
    m_shapeIndex = o.m_shapeIndex;

    m_volume = o.m_volume;

    m_worldPos = o.m_worldPos;
    m_quatRotation = o.m_quatRotation;
    m_degreesRotation = o.m_degreesRotation;
    m_viewUp = o.m_viewUp;
    m_viewFront = o.m_viewFront;
    m_viewRight = o.m_viewRight;
    m_modelMatrix = o.m_modelMatrix;
    m_modelScale = o.m_modelScale;

    return *this;
}

void Snapshot::updateDegrees() const noexcept
{
    ASSERT_RT();
    if (!m_dirtyDegrees) return;
    m_degreesRotation = util::quatToDegrees(m_quatRotation);
    m_dirtyDegrees = false;
}


void Snapshot::updateEntity(
    const UpdateContext& ctx,
    EntitySSBO* entity)
{
    ASSERT_RT();
    if (!m_dirtyEntity) return;

    entity->u_materialIndex = m_materialIndex;
    entity->u_shapeIndex = m_shapeIndex;

    entity->u_volume = m_volume;

    // NOTE KI M-T matrix needed *ONLY* if non uniform scale
    entity->setModelMatrix(m_modelMatrix, m_uniformScale, m_dirtyNormal);

    entity->u_worldScale = getWorldScale();

    m_dirtyEntity = false;
    m_dirtyNormal = false;
}
