#include "NodeGenerator.h"

#include "mesh/Transform.h"
#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"
#include "mesh/LodMeshInstance.h"

#include "model/Node.h"

#include "render/Batch.h"
#include "render/RenderContext.h"
#include "render/DrawableInfo.h"
#include "render/InstanceRegistry.h"

#include "registry/Registry.h"

NodeGenerator::~NodeGenerator() = default;

void NodeGenerator::registerDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container)
{
    auto entityIndex = container.getEntityIndex();

    const auto* snapshot = container.getSnapshotRT();

    const auto* type = container.getType();
    const auto& lodMeshes = type->getLodMeshes();
    const auto& lodMeshInstances = container.getLodMeshInstances();

    for (auto& transform : m_transforms) {
        for (int i = 0; i < lodMeshInstances.size(); i++) {
            const auto& lod = lodMeshInstances[i];
            const auto& lodMesh = lodMeshes[i];

            render::DrawableInfo drawable;
            {
                drawable.lodMeshIndex = i;
                drawable.meshId = lodMesh.getMesh<mesh::Mesh>()->m_id;

                drawable.entityIndex = entityIndex;
                drawable.materialIndex = lodMesh.m_materialIndex;
                drawable.jointBaseIndex = lod.m_jointBaseIndex;

                drawable.baseVertex = lodMesh.m_baseVertex;
                drawable.baseIndex = lodMesh.m_baseIndex;
                drawable.indexCount = lodMesh.m_indexCount;

                drawable.volume = transform.getVolume();
                drawable.localTransform = transform.getMatrix() * lodMesh.m_baseTransform;
                drawable.minDistance2 = lodMesh.m_minDistance2;
                drawable.maxDistance2 = lodMesh.m_maxDistance2;

                drawable.data = transform.getData();

                drawable.vaoId = lodMesh.m_vaoId;
                drawable.drawOptions = lodMesh.m_drawOptions;

                drawable.programId = lodMesh.m_programId;
                drawable.oitProgramId = lodMesh.m_oitProgramId;
                drawable.shadowProgramId = lodMesh.m_shadowProgramId;
                drawable.preDepthProgramId = lodMesh.m_preDepthProgramId;
                drawable.selectionProgramId = lodMesh.m_selectionProgramId;
                drawable.idProgramId = lodMesh.m_idProgramId;
                drawable.normalProgramId = lodMesh.m_normalProgramId;
            }

            auto index = instanceRegistry.registerDrawable(drawable);
            if (m_instanceRef.offset == 0) {
                m_instanceRef.offset = index;
            }
            m_instanceRef.size++;
        }
    }
}

void NodeGenerator::bindBatch(
    const render::RenderContext& ctx,
    const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
    const std::function<void(ki::program_id)>& programPrepare,
    uint8_t kindBits,
    render::Batch& batch,
    const model::Node& container)
{
    batch.addSnapshotsInstanced(
        ctx,
        container.getType(),
        m_instanceRef,
        programSelector,
        programPrepare,
        kindBits);
}
