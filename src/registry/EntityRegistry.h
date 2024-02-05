#pragma once

#include <bitset>
#include <vector>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

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
    static EntityRegistry& get() noexcept;

    EntityRegistry();
    EntityRegistry& operator=(const EntityRegistry&) = delete;

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

    inline void markDirty(int index) noexcept
    {
        //ASSERT_RT();
        if (index < m_minDirty || m_minDirty == -1) {
            m_minDirty = index;
        }
        if (index > m_maxDirty || m_maxDirty == -1) {
            m_maxDirty = index;
        }

        m_dirty[index] = true;
    }

private:
    void processNodes(const UpdateContext& ctx);

private:
    std::vector<EntitySSBO> m_entries;
    std::vector<bool> m_dirty;
    int m_minDirty = -1;
    int m_maxDirty = -1;

    std::atomic<bool> m_dirtyFlag;

    kigl::GLBuffer m_ssbo{ "entity_ssbo" };
    kigl::GLFence m_fence{ "fence_entity" };
    bool m_useMapped{ false };
    bool m_useInvalidate{ false };
    bool m_useFence{ false };
    bool m_useDebugFence{ false };
};
