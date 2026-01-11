#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include <glm/glm.hpp>

#include "util/BufferReference.h"
#include "util/DirtySet.h"
#include "util/SlotAllocator.h"

namespace editor
{
    class NodeTool;
}

namespace animation {
    inline const glm::mat4 ID_MAT{ 1.f };

    class AnimationSystem;
    class SocketBuffer;

    struct SocketTransformSSBO;

    class SocketRegistry {
        friend AnimationSystem;
        friend SocketBuffer;
        friend editor::NodeTool;

    public:
        SocketRegistry();
        SocketRegistry& operator=(const SocketRegistry&) = delete;

        ~SocketRegistry();

        void clear();
        void prepare();

        uint32_t getActiveCount() const noexcept;

        const glm::mat4& getTransform(
            uint32_t index) const noexcept
        {
            if (index <= 0 || index >= m_transforms.size()) return ID_MAT;
            return m_transforms[index];
        }

    protected:
        // Register node instance specific rig
        // @return instance index into socket transform buffer
        util::BufferReference allocate(size_t count);
        // @return null ref
        util::BufferReference release(util::BufferReference ref);

        std::span<const glm::mat4> getRange(
            const util::BufferReference ref) const noexcept;

        std::span<glm::mat4> modifyRange(
            util::BufferReference ref) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(util::BufferReference ref) noexcept;

        void updateWT();

    private:
        void makeSnapshot();

    protected :
        std::mutex m_lock{};

    private:
        std::atomic_bool m_updateReady{ false };

        std::vector<glm::mat4> m_transforms;

        util::SlotAllocator m_slotAllocator;
        util::DirtySet<util::BufferReference, true> m_dirtySlots;

        std::vector<SocketTransformSSBO> m_snapshot;
        std::vector<util::BufferReference> m_dirtySnapshot;
    };
}
