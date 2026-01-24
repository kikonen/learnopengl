#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <span>

#include <glm/glm.hpp>

#include "util/BufferReference.h"

namespace editor
{
    class NodeTool;
}

namespace animation {
    class AnimationSystem;
    class AnimateNode;

    class RigNodeRegistry {
        friend AnimationSystem;
        friend AnimateNode;
        friend editor::NodeTool;

    public:
        RigNodeRegistry();
        RigNodeRegistry& operator=(const RigNodeRegistry&) = delete;

        ~RigNodeRegistry();

        void clear();
        void prepare();

        uint32_t getActiveCount() const noexcept;

    protected:
        // Register node instance specific rig
        // @return instance index into joint transform buffer
        util::BufferReference allocate(size_t count);
        // @return null ref
        util::BufferReference release(util::BufferReference ref);

        std::span<const glm::mat4> getRange(util::BufferReference ref) const noexcept;

        std::span<glm::mat4> modifyRange(util::BufferReference ref) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(util::BufferReference ref) noexcept;

        void updateWT();

    protected:
        std::mutex m_lock{};

    private:
        std::mutex m_lockDirty{};

        std::atomic_bool m_updateReady{ false };

        std::vector<glm::mat4> m_transforms;

        std::unordered_map<util::BufferReference, bool> m_allocatedSlots;
        std::unordered_map<util::BufferReference, bool> m_dirtySlots;
        // { size: [index, ...] }
        std::unordered_map<uint32_t, std::vector<uint32_t>> m_freeSlots;

    };
}
