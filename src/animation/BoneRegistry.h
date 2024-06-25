#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

namespace animation {
    class BoneRegistry {
    public:
        static animation::BoneRegistry& get() noexcept;

        BoneRegistry();
        BoneRegistry& operator=(const BoneRegistry&) = delete;

        ~BoneRegistry();

        void prepare();

        // Register node instance specific rig
        // @return instance index into bone transform buffer
        uint32_t reserveInstance(size_t count);

        std::span<glm::mat4> modifyRange(
            uint32_t start,
            size_t count) noexcept;

        void markDirtyAll() noexcept;
        void markDirty(size_t start, size_t count) noexcept;

        uint32_t getActiveCount() const noexcept;

        void updateWT();
        void updateRT();

    private:
        void makeSnapshot();
        void updateBuffer();

        bool updateSpan(
            size_t updateIndex,
            size_t updateCount);

    private:
        std::mutex m_lock{};

        std::atomic_bool m_updateReady{ false };
        size_t m_frameSkipCount{ 0 };

        std::vector<glm::mat4> m_transforms;
        std::vector<std::pair<uint32_t, size_t>> m_dirtyTransform;

        std::vector<glm::mat4> m_snapshot;
        std::vector<std::pair<uint32_t, size_t>> m_dirtySnapshot;

        kigl::GLBuffer m_ssbo{ "bone_palette_ssbo" };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}
