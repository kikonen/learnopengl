#include "EntityRegistry.h"

#include "EntitySSBO.h"

#include "scene/RenderContext.h"


namespace {
    // scene_full = 91 109
    constexpr int MAX_ENTITY_COUNT = 500000;

    constexpr int MAX_SKIP = 10;
}

EntityRegistry::EntityRegistry(const Assets& assets)
    : m_assets(assets)
{
    m_entries.reserve(MAX_ENTITY_COUNT);
}

void EntityRegistry::prepare()
{
    m_ssbo.createEmpty(MAX_ENTITY_COUNT * sizeof(EntitySSBO), GL_DYNAMIC_STORAGE_BIT);
}

void EntityRegistry::update(const RenderContext& ctx)
{
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
            const int count = (to + 1) - from;
            //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, from={}, to={}, count={}", ctx.m_clock.frameCount, from, to, count));
            m_ssbo.update(
                from * sz,
                (to + 1 - from) * sz,
                &m_entries[from]);

            skip = 0;
            from = -1;
            updatedCount += count;
        }
        idx++;
    }

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, updated={}", ctx.m_clock.frameCount, updatedCount));

    for (int i = m_minDirty; i < m_maxDirty; i++) {
        m_dirty[i] = false;
    }

    m_minDirty = -1;
    m_maxDirty = -1;
}

void EntityRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_ENTITIES);
}

// const EntitySSBO& entry
int EntityRegistry::add()
{
    if (m_entries.size() == MAX_ENTITY_COUNT)
        throw std::runtime_error{ "MAX_ENTITY_COUNT" };

    m_entries.emplace_back();
    m_dirty.emplace_back(true);

    const auto index = m_entries.size() - 1;
    markDirty(index);

    return index;
}

EntitySSBO* EntityRegistry::get(int index)
{
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
