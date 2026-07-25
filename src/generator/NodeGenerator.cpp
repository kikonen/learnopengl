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
    const auto& lodMeshes = container.getEnabledMeshes();
    const auto& lodMeshInstances = container.getLodMeshInstances();

    uint32_t groupId = 0;

    m_instanceRef = instanceRegistry.allocate(m_transforms.size() * lodMeshes.size());
    auto drawables = instanceRegistry.modifyRange(m_instanceRef);
    int drawableIndex = 0;

    for (auto& transform : m_transforms) {
        for (int i = 0; i < lodMeshInstances.size(); i++) {
            const auto& lod = lodMeshInstances[i];
            const auto& lodMesh = *lodMeshes[i];

            auto& drawable = drawables[drawableIndex++];
            {
                drawable.meshId = lodMesh.getMesh<mesh::Mesh>()->getId();
                drawable.groupId = groupId;

                drawable.entityIndex = entityIndex;
                drawable.materialIndex = lodMesh.getMaterialIndex();
                drawable.jointBaseIndex = lod.m_jointBaseIndex;

                drawable.baseVertex = lodMesh.getBaseVertex();
                drawable.baseIndex = lodMesh.getBaseIndex();
                drawable.indexCount = lodMesh.getIndexCount();

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

                drawable.m_ignoredBy = container.m_ignoredBy;
                drawable.m_flags = render::toDrawableFlags(container.m_typeFlags, lodMesh.m_flags.noShadow);
                drawable.m_flags.hidden = !container.m_visible;
            }
        }

        groupId++;
    }

    instanceRegistry.prepareInstances(m_instanceRef);
}

void NodeGenerator::updateDrawables(
    render::InstanceRegistry& instanceRegistry,
    const model::Node& container,
    const model::Snapshot& snapshot)
{
    ASSERT_RT();

    if (m_instanceRef.empty()) return;
    if (m_dirtySlots.empty()) return;

    const auto* type = container.getType();
    const auto& lodMeshes = container.getEnabledMeshes();

    auto drawables = instanceRegistry.modifyRange(m_instanceRef);
    {
        // NOTE KI m_dirtySlots store TRANSFORM-local ranges (0-based per generator).
        // The drawable array holds meshCount entries per transform laid out as
        // [transformIdx * meshCount + meshIdx] (see registerDrawables). InstanceRegistry
        // treats refs as ABSOLUTE offsets, so the dirty range must be scaled by meshCount
        // and shifted by m_instanceRef.offset before handing it back.
        const size_t meshCount = lodMeshes.size();

        for (const auto& dirtyRef : m_dirtySlots) {
            for (size_t transformIndex = dirtyRef.offset;
                transformIndex < dirtyRef.offset + dirtyRef.size;
                transformIndex++)
            {
                const auto& transform = m_transforms[transformIndex];
                for (size_t i = 0; i < meshCount; i++) {
                    const auto& lodMesh = *lodMeshes[i];
                    auto& drawable = drawables[transformIndex * meshCount + i];

                    drawable.worldVolume = transform.getWorldVolume();
                    drawable.localTransform = transform.getMatrix() * lodMesh.m_baseTransform;
                }
            }

            const util::BufferReference instanceRef{
                m_instanceRef.offset + dirtyRef.offset * meshCount,
                dirtyRef.size * meshCount
            };

            instanceRegistry.markDirty(instanceRef);
            instanceRegistry.updateInstances(instanceRef);
        }
    }

    m_dirtySlots.clear();
}

void NodeGenerator::releaseInstances(render::InstanceRegistry& instanceRegistry)
{
    instanceRegistry.release(m_instanceRef);
    m_instanceRef = {};
}

void NodeGenerator::markDirty(
    const util::BufferReference ref)
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
