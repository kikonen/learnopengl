#include "EntityRegistry.h"

#include "util/thread.h"

#include "EntitySSBO.h"

#include "engine/UpdateContext.h"

#include "render/RenderContext.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace {
    constexpr size_t ENTITY_BLOCK_SIZE = 100;
    constexpr size_t ENTITY_BLOCK_COUNT = 20000;

    constexpr size_t MAX_ENTITY_COUNT = ENTITY_BLOCK_SIZE * ENTITY_BLOCK_COUNT;

    constexpr size_t MAX_SKIP = 20;
}

EntityRegistry::EntityRegistry(const Assets& assets)
    : m_assets(assets)
{
    // HACK KI reserve nax to avoid memory alloc issue main vs. worker
    m_entries.reserve(MAX_ENTITY_COUNT);
}

void EntityRegistry::prepare()
{
    m_debugFence = m_assets.batchDebug;

    // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
    m_ssbo.createEmpty(ENTITY_BLOCK_SIZE * sizeof(EntitySSBO), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT);
    m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
}

void EntityRegistry::updateWT(const UpdateContext& ctx)
{
    //std::lock_guard<std::mutex> lock(m_lock);
}

void EntityRegistry::updateRT(const UpdateContext& ctx)
{
    //if (!m_dirty) return;
    //std::lock_guard<std::mutex> lock(m_lock);
    processNodes(ctx);

    if (m_minDirty < 0) return;

    if (m_assets.glUseFence) {
        m_fence.waitFence(m_debugFence);
    }

    constexpr size_t sz = sizeof(EntitySSBO);
    const int maxCount = (m_maxDirty + 1) - m_minDirty;

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, min={}, max={}, maxCount={}", ctx.m_clock.frameCount, m_minDirty, m_maxDirty, maxCount));

    int idx = m_minDirty;
    int from = -1;
    int skip = 0;

    const size_t totalCount = m_entries.size();

    bool refreshAll = false;
    {
        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < totalCount * sz) {
            m_ssbo.resizeBuffer(m_entries.capacity() * sz);
            m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
            refreshAll = true;
        }
    }

    int updatedCount = 0;

    if (refreshAll) {
        memcpy(m_ssbo.m_data, &m_entries[0], totalCount * sz);
        m_ssbo.flushRange(0, totalCount * sz);
        for (int i = 0; i < totalCount; i++) {
            m_dirty[i] = false;
        }
    }
    else {
        while (idx <= m_maxDirty) {
            if (!m_dirty[idx]) {
                skip++;
            }
            else {
                if (from == -1) from = idx;
            }

            if (from != -1 && (skip >= MAX_SKIP || idx == m_maxDirty)) {
                int to = idx;
                const int count = to + 1 - from;

                //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, from={}, to={}, count={}", ctx.m_clock.frameCount, from, to, count));
                memcpy(m_ssbo.m_data + from * sz, &m_entries[from], count * sz);

                m_ssbo.flushRange(from * sz, count * sz);

                for (int i = 0; i < count; i++) {
                    m_dirty[from + i] = false;
                }

                skip = 0;
                from = -1;
                updatedCount += count;
            }
            idx++;
        }
    }

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, updated={}", ctx.m_clock.frameCount, updatedCount));

    m_minDirty = -1;
    m_maxDirty = -1;
}

void EntityRegistry::postRT(const UpdateContext& ctx)
{
    // NOTE KI if there was no changes old fence is stil valid
    if (m_assets.glUseFence) {
        if (!m_fence.isSet()) {
            m_fence.setFence(m_debugFence);
        }
    }
}

void EntityRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_ENTITIES);
}

// index of entity
int EntityRegistry::registerEntity()
{
    return registerEntityRange(1);
}

// @return first index of range
int EntityRegistry::registerEntityRange(const size_t count)
{
    ASSERT_RT();

    //std::lock_guard<std::mutex> lock(m_lock);

    if (m_entries.size() + count > MAX_ENTITY_COUNT)
        throw std::runtime_error{ fmt::format("MAX_ENTITY_COUNT: {}", MAX_ENTITY_COUNT) };

    {
        size_t size = m_entries.size() + std::max(ENTITY_BLOCK_SIZE, count) + ENTITY_BLOCK_SIZE;
        size = std::min(size, MAX_ENTITY_COUNT);
        m_entries.reserve(size);
    }

    size_t firstIndex = m_entries.size();

    for (int i = 0; i < count; i++) {
        m_entries.emplace_back();
        m_dirty.emplace_back(false);

        // NOTE KI *lazy*, allow generating entities, without updating GPU side
        //markDirty(firstIndex + i);
    }

    KI_INFO(fmt::format("Entity: ADDED_RANGE: firstIndex={}, count={}, newSize={}", firstIndex, count, m_entries.size()));

    return static_cast<int>(firstIndex);
}

const EntitySSBO* EntityRegistry::getEntity(int index) const
{
    return &m_entries[index];
}

EntitySSBO* EntityRegistry::modifyEntity(int index, bool dirty)
{
    if (dirty) markDirty(index);
    return &m_entries[index];
}

void EntityRegistry::markDirty(int index)
{
    if (index < m_minDirty || m_minDirty == -1) {
        m_minDirty = index;
    }
    if (index > m_maxDirty || m_maxDirty == -1) {
        m_maxDirty = index;
    }

    m_dirty[index] = true;
}

void EntityRegistry::processNodes(const UpdateContext& ctx)
{
    for (auto* node : ctx.m_registry->m_nodeRegistry->m_allNodes) {
        node->updateEntity(ctx, this);
    }
}
