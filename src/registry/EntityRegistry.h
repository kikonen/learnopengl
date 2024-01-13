#pragma once

#include <bitset>
#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "asset/Assets.h"
#include "asset/SSBO.h"

#include "EntitySSBO.h"

struct UpdateContext;
class RenderContext;

//
// Manage SSBO buffer for registered entity instances
// - allow binding all matrices at *once*, and update them
//   once per frame instead of pushing them multiple times into dynamic VBOs
//
class EntityRegistry {
public:
    EntityRegistry(const Assets& assets);

    void prepare();
    void updateRT(const UpdateContext& ctx);
    void postRT(const UpdateContext& ctx);
    void bind(const RenderContext& ctx);

    // index of entity
    uint32_t registerEntity();

    // @return first index of range
    uint32_t registerEntityRange(const size_t count);

    const EntitySSBO* getEntity(int index) const noexcept
    {
        return &m_entries[index];
    }

    inline const std::span<EntitySSBO> getEntityRange(uint32_t start, uint32_t count) noexcept {
        return std::span{ m_entries }.subspan(start, count);
    }

    EntitySSBO* modifyEntity(int index, bool dirty)
    {
        if (dirty) markDirty(index);
        return &m_entries[index];
    }

    inline std::span<EntitySSBO> modifyEntityRange(uint32_t start, uint32_t count) noexcept {
        return std::span{ m_entries }.subspan(start, count);
    }

    void markDirty(int index);

private:
    void processNodes(const UpdateContext& ctx);

private:
    const Assets& m_assets;

    std::vector<EntitySSBO> m_entries;
    std::vector<bool> m_dirty;
    int m_minDirty = -1;
    int m_maxDirty = -1;

    std::atomic<bool> m_dirtyFlag;
    std::mutex m_lock{};

    kigl::GLBuffer m_ssbo{ "entity_ssbo" };
    kigl::GLFence m_fence{ "fence_entity" };
    bool m_mappedMode{ false };
    bool m_debugFence{ false };
};
