#include "NodeGenerator.h"

#include "asset/AABB.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "render/Batch.h"

#include "engine/UpdateContext.h"

#include "registry/EntityRegistry.h"

void NodeGenerator::snapshotWT(bool force)
{
    const auto diff = m_transforms.size() - m_snapshotsWT.size();
    if (diff != 0) {
        force |= true;
        m_snapshotsWT.reserve(m_transforms.size());
        for (int i = 0; i < diff; i++) {
            m_snapshotsWT.emplace_back();
        }
    }

    for (size_t i = 0; i < m_transforms.size(); i++) {
        auto& transform = m_transforms[i];
        assert(!transform.m_dirty);
        if (!force && !transform.m_dirtySnapshot) continue;
        m_snapshotsWT[i] = transform;
        m_snapshotsWT[i].m_dirty = true;
        transform.m_dirtySnapshot = false;
        transform.m_dirtyNormal = false;
    }
}

void NodeGenerator::snapshotRT(bool force)
{
    const auto diff = m_snapshotsWT.size() - m_snapshotsRT.size();
    if (diff != 0) {
        force |= true;
        m_snapshotsRT.reserve(m_snapshotsWT.size());
        for (int i = 0; i < diff; i++) {
            m_snapshotsRT.emplace_back();
        }
    }

    for (size_t i = 0; i < m_snapshotsWT.size(); i++) {
        if (!force && !m_snapshotsWT[i].m_dirty) continue;
        m_snapshotsRT[i] = m_snapshotsWT[i];
        m_snapshotsWT[i].m_dirty = false;
    }
}

void NodeGenerator::updateEntity(
    const UpdateContext& ctx,
    Node& container,
    EntityRegistry* entityRegistry,
    bool force)
{
    if (m_activeCount == 0) return;

    int entityIndex = m_reservedFirst;

    for (auto& snapshot : m_snapshotsRT) {
        if (!force && !snapshot.m_dirtyEntity) continue;
        if (snapshot.m_entityIndex == -1) continue;

        auto* entity = entityRegistry->modifyEntity(snapshot.m_entityIndex, true);

        entity->u_objectID = container.m_id;
        entity->u_flags = container.getEntityFlags();
        entity->u_highlightIndex = container.getHighlightIndex(ctx.m_assets);

        snapshot.updateEntity(ctx, entity);

        entityIndex++;
    }
}

void NodeGenerator::bindBatch(
    const RenderContext& ctx,
    Node& container,
    render::Batch& batch)
{
    if (m_activeCount == 0) return;

    // NOTE KI instanced node may not be ready, or currently not generating visible entities
    //batch.addInstanced(ctx, container.getEntityIndex(), m_activeFirst, m_activeCount);
    batch.addInstanced(ctx, -1, m_activeFirst, m_activeCount);
}

const kigl::GLVertexArray* NodeGenerator::getVAO(
    const Node& container) const noexcept
{
    return container.m_type->getVAO();
}

const backend::DrawOptions& NodeGenerator::getDrawOptions(
    const Node& container) const noexcept
{
    return container.m_type->getDrawOptions();
}

const glm::vec4 NodeGenerator::calculateVolume()
{
    AABB minmax{ true };

    for (auto& transform : m_transforms)
    {
        minmax.minmax(transform.getPosition());
    }

    return minmax.getVolume();
}
