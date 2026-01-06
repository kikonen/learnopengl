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
    constexpr size_t BLOCK_SIZE = 1000;

    //constexpr size_t MAX_ENTITY_COUNT = ENTITY_BLOCK_SIZE * MAX_ENTITY_BLOCK_COUNT;

    constexpr size_t MAX_SKIP = 20;

    static EntityRegistry* s_registry{ nullptr };
}

void EntityRegistry::init() noexcept
{
    assert(!s_registry);
    s_registry = new EntityRegistry();
}

void EntityRegistry::release() noexcept
{
    auto* s = s_registry;
    s_registry = nullptr;
    delete s;
}

EntityRegistry& EntityRegistry::get() noexcept
{
    assert(s_registry);
    return *s_registry;
}

EntityRegistry::EntityRegistry()
{
}

EntityRegistry::~EntityRegistry() = default;

void EntityRegistry::clear()
{
    ASSERT_RT();

    m_ssbo.markUsed(0);
}

void EntityRegistry::prepare()
{
    ASSERT_RT();

    const auto& assets = Assets::get();

    // https://stackoverflow.com/questions/44203387/does-gl-map-invalidate-range-bit-require-glinvalidatebuffersubdata
    GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    m_ssbo.createEmpty(BLOCK_SIZE * sizeof(EntitySSBO), flags);
    m_ssbo.map(flags);

    m_ssbo.bindSSBO(SSBO_ENTITIES);
}

void EntityRegistry::updateRT(const UpdateContext& ctx)
{
    auto [minDirty, maxDirty] = ctx.getRegistry()->m_nodeRegistry->updateEntity(ctx);

    if (minDirty > maxDirty) return;

    constexpr size_t sz = sizeof(EntitySSBO);
    const int maxCount = (maxDirty + 1) - minDirty;

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, min={}, max={}, maxCount={}", ctx.getClock().frameCount, m_minDirty, m_maxDirty, maxCount));

    int idx = minDirty;
    int from = -1;
    int skip = 0;

    auto& entries = NodeRegistry::get().getEntities();
    auto& dirtyEntries = NodeRegistry::get().getDirtyEntities();
    const size_t totalCount = entries.size();

    resizeBuffer(totalCount);

    int updatedCount = 0;

    if (false) {
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

                //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, from={}, to={}, count={}", ctx.getClock().frameCount, from, to, count));
                std::copy(
                    std::begin(entries) + from,
                    std::begin(entries) + count,
                    m_ssbo.mapped<EntitySSBO>(from * sz));

                //m_ssbo.flushRange(from * sz, count * sz);

                skip = 0;
                from = -1;
                updatedCount += count;
            }
            idx++;
        }
    }
    else {
        std::copy(
            std::begin(entries) + 0,
            std::begin(entries) + totalCount,
            m_ssbo.mapped<EntitySSBO>(0));
    }

    m_ssbo.markUsed(totalCount * sz);

    for (int i = 0; i < totalCount; i++) {
        dirtyEntries[i] = false;
    }

    //KI_DEBUG(fmt::format("ENTITY_UPDATE: frame={}, updated={}", ctx.getClock().frameCount, updatedCount));
}

void EntityRegistry::resizeBuffer(size_t totalCount)
{
    constexpr auto sz = sizeof(EntitySSBO);

    if (m_ssbo.size() >= totalCount * sz) return;

    size_t blocks = (totalCount / BLOCK_SIZE) + 2;
    size_t bufferSize = blocks * BLOCK_SIZE * sz;

    m_ssbo.resizeBuffer(bufferSize, true);

    GLuint flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
    m_ssbo.map(flags);

    m_ssbo.bindSSBO(SSBO_ENTITIES);
}

void EntityRegistry::beginFrame() {
    m_fence.waitFence();
}

void EntityRegistry::endFrame() {
    m_fence.setFence();
}

