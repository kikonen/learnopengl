#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

namespace editor
{
    class NodeTool;
}

namespace animation {
    class AnimationSystem;
    class JointBuffer;

    struct JointTransformSSBO;

    class JointRegistry {
        friend JointBuffer;
        friend AnimationSystem;
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
        uint32_t addInstance(size_t count);
        void removeInstance(
            uint32_t index,
            size_t count);

        std::span<glm::mat4> modifyRange(
            uint32_t start,
            size_t count) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(size_t start, size_t count) noexcept;

        void updateWT();

    private:
        void makeSnapshot();

    protected:
        std::mutex m_lock{};

    private:
        std::mutex m_lockDirty{};

        std::atomic_bool m_updateReady{ false };

        // { size: [index, ...] }
        std::unordered_map<size_t, std::vector<uint32_t>> m_freeSlots;

        std::vector<glm::mat4> m_transforms;
        std::vector<std::pair<uint32_t, size_t>> m_dirtyTransform;

        std::vector<JointTransformSSBO> m_snapshot;
        std::vector<std::pair<uint32_t, size_t>> m_dirtySnapshot;
    };
}
