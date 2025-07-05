#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

namespace animation {
    class AnimationSystem;
    class BoneBuffer;

    class BoneRegistry {
        friend BoneBuffer;
        friend AnimationSystem;

    public:
        BoneRegistry();
        BoneRegistry& operator=(const BoneRegistry&) = delete;

        ~BoneRegistry();

        void clear();

        void shutdown();
        void prepare();

        uint32_t getActiveCount() const noexcept;

    protected:
        // Register node instance specific rig
        // @return instance index into bone transform buffer
        uint32_t addInstance(size_t count);
        void removeInstance(uint32_t index, size_t count);

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

        std::vector<glm::mat4> m_transforms;
        std::vector<std::pair<uint32_t, size_t>> m_dirtyTransform;

        std::vector<glm::mat4> m_snapshot;
        std::vector<std::pair<uint32_t, size_t>> m_dirtySnapshot;
    };
}
