#include "NodeGenerator.h"

#include "mesh/Transform.h"

#include "model/Node.h"

#include "render/Batch.h"


NodeGenerator::~NodeGenerator() = default;

void NodeGenerator::bindBatch(
    const render::RenderContext& ctx,
    const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
    const std::function<void(ki::program_id)>& programPrepare,
    uint8_t kindBits,
    render::Batch& batch,
    const model::Node& container,
    const model::Snapshot& snapshot)
{
    batch.addSnapshotsInstanced(
        ctx,
        container.getType(),
        container.getLodMeshes(),
        container.getLodMeshInstances(),
        programSelector,
        programPrepare,
        kindBits,
        snapshot,
        m_transforms,
        container.getEntityIndex());
}
