#include "NodeGenerator.h"

#include "util/thread.h"

#include "mesh/Transform.h"
#include "mesh/Mesh.h"
#include "mesh/LodMesh.h"
#include "mesh/LodMeshInstance.h"

#include "model/Node.h"
#include "model/Snapshot.h"

#include "render/Batch.h"
#include "render/RenderContext.h"
#include "render/DrawableInfo.h"
#include "render/InstanceRegistry.h"

#include "registry/Registry.h"

NodeGenerator::~NodeGenerator() = default;

void NodeGenerator::registerDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container,
    const model::Snapshot& snapshot)
{
    ASSERT_RT();

    auto entityIndex = container.getEntityIndex();

    const auto* type = container.getType();
    const auto& lodMeshes = type->getLodMeshes();
    const auto& lodMeshInstances = container.getLodMeshInstances();

    uint32_t groupId = 0;

    m_instanceRef = instanceRegistry.allocate(m_transforms.size() * lodMeshes.size());
    auto drawables = instanceRegistry.modifyRange(m_instanceRef);
    int drawableIndex = 0;

    for (auto& transform : m_transforms) {
        for (int i = 0; i < lodMeshInstances.size(); i++) {
            const auto& lod = lodMeshInstances[i];
            const auto& lodMesh = lodMeshes[i];

            auto& drawable = drawables[drawableIndex++];
            {
                drawable.lodMeshIndex = i;
                drawable.meshId = lodMesh.getMesh<mesh::Mesh>()->m_id;
                drawable.groupId = groupId;

                drawable.entityIndex = entityIndex;
                drawable.materialIndex = lodMesh.m_materialIndex;
                drawable.jointBaseIndex = lod.m_jointBaseIndex;

                drawable.baseVertex = lodMesh.m_baseVertex;
                drawable.baseIndex = lodMesh.m_baseIndex;
                drawable.indexCount = lodMesh.m_indexCount;

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

                // TODO KI volume/transform can change per frame
                drawable.worldVolume = transform.getWorldVolume();
                drawable.localTransform = transform.getMatrix() * lodMesh.m_baseTransform;
            }
        }

        groupId++;
    }

    instanceRegistry.prepareInstances(m_instanceRef);
}

void NodeGenerator::updateDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container)
{
    ASSERT_RT();

    if (m_instanceRef.empty()) return;
    if (m_dirtySlots.empty()) return;

    const auto* type = container.getType();
    const auto& lodMeshes = type->getLodMeshes();

    auto drawables = instanceRegistry.modifyRange(m_instanceRef);
    int drawableIndex = 0;

    for (auto& transform : m_transforms) {
        for (int i = 0; i < lodMeshes.size(); i++) {
            const auto& lodMesh = lodMeshes[i];
            auto& drawable = drawables[drawableIndex++];

            drawable.worldVolume = transform.getWorldVolume();
            drawable.localTransform = transform.getMatrix() * lodMesh.m_baseTransform;
        }
    }

    instanceRegistry.markDirty(m_instanceRef);
    instanceRegistry.updateInstances(m_instanceRef);

    m_dirtySlots.clear();
}

void NodeGenerator::bindBatch(
    const render::RenderContext& ctx,
    const std::function<ki::program_id (const render::DrawableInfo&)>& programSelector,
    const std::function<void(ki::program_id)>& programPrepare,
    uint8_t kindBits,
    render::Batch& batch,
    const model::Node& container)
{
    batch.addDrawablesInstanced(
        ctx,
        container.getType(),
        m_instanceRef,
        programSelector,
        programPrepare,
        kindBits);
}

void NodeGenerator::markDirty(util::BufferReference ref)
{
    //ASSERT_WT();
    if (ref.size == 0) return;

    const auto& it = std::find_if(
        m_dirtySlots.begin(),
        m_dirtySlots.end(),
        [&ref](const auto& old) {
        return old.contains(ref);
    });
    if (it != m_dirtySlots.end()) return;

    m_dirtySlots.push_back(ref);
}
