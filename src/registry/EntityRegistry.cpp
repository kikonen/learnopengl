#include "EntityRegistry.h"

#include "util/thread.h"

#include "asset/Assets.h"

#include "EntitySSBO.h"

#include "engine/UpdateContext.h"

#include "render/RenderContext.h"

#include "model/Node.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace {
    constexpr size_t ENTITY_BLOCK_SIZE = 100;
    constexpr size_t ENTITY_BLOCK_COUNT = 40000;

    constexpr size_t MAX_ENTITY_COUNT = ENTITY_BLOCK_SIZE * ENTITY_BLOCK_COUNT;

    constexpr size_t MAX_SKIP = 20;
}

EntityRegistry& EntityRegistry::get() noexcept
{
    static EntityRegistry s_registry;
    return s_registry;
}

EntityRegistry::EntityRegistry()
{
}

void EntityRegistry::prepare()
{
    const auto& assets = Assets::get();

    m_useMapped = assets.glUseMapped;
    m_useInvalidate = assets.glUseInvalidate;
    m_useFence = assets.glUseFence;
    m_useDebugFence = assets.glUseDebugFence;

    m_useMapped = false;
    m_useInvalidate = true;
    m_useFence = false;
    m_useDebugFence = false;

    if (m_useMapped) {
        // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
        m_ssbo.createEmpty(ENTITY_BLOCK_SIZE * sizeof(EntitySSBO), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
    }
    else {
        m_ssbo.createEmpty(ENTITY_BLOCK_SIZE * sizeof(EntitySSBO), GL_DYNAMIC_STORAGE_BIT);
    }
}

void EntityRegistry::updateRT(const UpdateContext& ctx)
{
    auto [minDirty, maxDirty] = ctx.m_registry->m_nodeRegistry->updateEntity(ctx);

    if (minDirty > maxDirty) return;

    if (m_useFence) {
        m_fence.waitFence(m_useDebugFence);
    }

    constexpr size_t sz = sizeof(EntitySSBO);
    const int maxCount = (maxDirty + 1) - minDirty;

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, min={}, max={}, maxCount={}", ctx.m_clock.frameCount, m_minDirty, m_maxDirty, maxCount));

    int idx = minDirty;
    int from = -1;
    int skip = 0;

    auto& entries = NodeRegistry::get().getEntities();
    auto& dirtyEntries = NodeRegistry::get().getDirtyEntities();
    const size_t totalCount = entries.size();

    bool refreshAll = false;
    {
        // NOTE KI *reallocate* SSBO if needed
        if (m_ssbo.m_size < totalCount * sz) {
            m_ssbo.resizeBuffer(entries.capacity() * sz);
            if (m_useMapped) {
                m_ssbo.map(GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_FLUSH_EXPLICIT_BIT);
            }
            else {
                m_ssbo.bindSSBO(SSBO_ENTITIES);
            }
            refreshAll = true;
        }
    }

    int updatedCount = 0;

    if (refreshAll) {
        if (m_useMapped) {
            memcpy(m_ssbo.m_data, entries.data(), totalCount * sz);
            m_ssbo.flushRange(0, totalCount * sz);
        }
        else {
            //if (m_useInvalidate) {
            //    m_ssbo.invalidateRange(0, totalCount * sz);
            //}
            m_ssbo.update(0, totalCount * sz, entries.data());
        }
    }
    else {
        while (idx <= maxDirty) {
            if (!dirtyEntries[idx]) {
                skip++;
            }
            else {
                if (from == -1) from = idx;
            }

            if (from != -1 && (skip >= MAX_SKIP || idx == maxDirty)) {
                int to = idx;
                const int count = to + 1 - from;

                //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, from={}, to={}, count={}", ctx.m_clock.frameCount, from, to, count));
                if (m_useMapped) {
                    memcpy(m_ssbo.m_data + from * sz, &entries[from], count * sz);
                    m_ssbo.flushRange(from * sz, count * sz);
                }
                else {
                    //if (m_useInvalidate) {
                    //    m_ssbo.invalidateRange(from * sz, count * sz);
                    //}
                    m_ssbo.update(from * sz, count * sz, &entries[from]);
                }

                skip = 0;
                from = -1;
                updatedCount += count;
            }
            idx++;
        }
    }

    for (int i = 0; i < totalCount; i++) {
        dirtyEntries[i] = false;
    }

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, updated={}", ctx.m_clock.frameCount, updatedCount));
}

void EntityRegistry::postRT(const UpdateContext& ctx)
{
    // NOTE KI if there was no changes old fence is stil valid
    if (m_useFence) {
        if (!m_fence.isSet()) {
            m_fence.setFence(m_useDebugFence);
        }
    }
}

void EntityRegistry::bind(
    const RenderContext& ctx)
{
    m_ssbo.bindSSBO(SSBO_ENTITIES);
}
