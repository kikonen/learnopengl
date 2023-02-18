#include "EntityRegistry.h"

#include "EntitySSBO.h"

#include "scene/RenderContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace {
    // scene_full = 91 109
    constexpr int MAX_ENTITY_COUNT = 2000000;

    constexpr int MAX_SKIP = 20;
}

EntityRegistry::EntityRegistry(const Assets& assets)
    : m_assets(assets)
{
    m_entries.reserve(MAX_ENTITY_COUNT);
}

void EntityRegistry::prepare()
{
    m_ssbo.createEmpty(MAX_ENTITY_COUNT * sizeof(EntitySSBO), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT);
    m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
}

void EntityRegistry::update(const RenderContext& ctx)
{
    processNodes(ctx);

    if (m_minDirty < 0) return;

    constexpr size_t sz = sizeof(EntitySSBO);
    const int maxCount = (m_maxDirty + 1) - m_minDirty;

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, min={}, max={}, maxCount={}", ctx.m_clock.frameCount, m_minDirty, m_maxDirty, maxCount));

    int idx = m_minDirty;
    int from = -1;
    int skip = 0;

    const int size = m_dirty.size();
    int updatedCount = 0;

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

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, updated={}", ctx.m_clock.frameCount, updatedCount));

    //for (int i = m_minDirty; i < m_maxDirty; i++) {
    //    m_dirty[i] = false;
    //}

    m_minDirty = -1;
    m_maxDirty = -1;
}

void EntityRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_ENTITIES);
}

// index of entity
int EntityRegistry::addEntity()
{
    return addEntityRange(1);
}

// @return first index of range
int EntityRegistry::addEntityRange(const int count)
{
    if (m_entries.size() + count > MAX_ENTITY_COUNT)
        throw std::runtime_error{ "MAX_ENTITY_COUNT" };

    int firstIndex = m_entries.size();

    for (int i = 0; i < count; i++) {
        m_entries.emplace_back();
        m_dirty.emplace_back(true);

        markDirty(firstIndex + i);
    }

    return firstIndex;
}

EntitySSBO* EntityRegistry::getEntity(int index)
{
    return &m_entries[index];
}

EntitySSBO* EntityRegistry::updateEntity(int index, bool dirty)
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

void EntityRegistry::processNodes(const RenderContext& ctx)
{
    for (const auto& all : ctx.m_registry->m_nodeRegistry->allNodes) {
        for (const auto& it : all.second) {
            for (auto& node : it.second) {
                node->updateEntity(ctx);
            }
        }
    }
}

