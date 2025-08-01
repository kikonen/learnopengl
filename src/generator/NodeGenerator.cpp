#include "NodeGenerator.h"

#include "mesh/Transform.h"

#include "model/Node.h"

#include "render/Batch.h"


NodeGenerator::~NodeGenerator() = default;

void NodeGenerator::bindBatch(
    const RenderContext& ctx,
    const std::function<ki::program_id (const mesh::LodMesh&)>& programSelector,
    const std::function<void(ki::program_id)>& programPrepare,
    uint8_t kindBits,
    render::Batch& batch,
    const Node& container,
    const Snapshot& snapshot)
{
    batch.addSnapshotsInstanced(
        ctx,
        &container,
        programSelector,
        programPrepare,
        kindBits,
        snapshot,
        m_transforms,
        container.m_entityIndex);
}
