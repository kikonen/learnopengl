#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

#include "util/BufferReference.h"
#include "util/DirtySet.h"
#include "util/SlotAllocator.h"

namespace editor
{
    class NodeTool;
}

namespace animation {
    class AnimationSystem;
    class AnimateNode;
    class JointBuffer;
    struct PaletteReference;

    struct JointTransformSSBO;

    class JointRegistry {
        friend JointBuffer;
        friend AnimationSystem;
        friend AnimateNode;
        friend editor::NodeTool;

    public:
        JointRegistry();
        JointRegistry& operator=(const JointRegistry&) = delete;

        ~JointRegistry();

        void clear();
        void prepare();

        uint32_t getActiveCount() const noexcept;

    protected:
        // Register node instance specific rig
        // @return instance index into joint transform buffer
        util::BufferReference allocate(size_t count);
        // @return null ref
        util::BufferReference release(util::BufferReference ref);

		std::span<const glm::mat4> getRange(
			const util::BufferReference ref) const noexcept;

        std::span<glm::mat4> modifyRange(
            util::BufferReference ref) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(util::BufferReference ref) noexcept;

    private:
        void makeSnapshot();

    protected:
        std::mutex m_lock{};

    private:
        std::atomic_bool m_updateReady{ false };

        std::vector<glm::mat4> m_transforms;

        util::SlotAllocator m_slotAllocator;
        util::DirtySet<util::BufferReference, true> m_dirtySlots;

        std::vector<JointTransformSSBO> m_snapshot;
        std::vector<util::BufferReference> m_dirtySnapshot;
    };
}
