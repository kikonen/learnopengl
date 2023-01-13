#include "EntityRegistry.h"

#include "EntitySSBO.h"


namespace {
    // scene_full = 91 109
    constexpr int MAX_ENTITY_COUNT = 500000;
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
    const int count = (m_maxDirty + 1) - m_minDirty;

    m_ssbo.update(
        m_minDirty * sz,
        count * sz,
        &m_entries[m_minDirty]);

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
