#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>
#include <memory>

#include "kigl/GLSyncQueue.h"

namespace animation {
    class AnimationSystem;
    class BoneRegistry;

    struct BoneTransformSSBO;

    class BoneBuffer {
        friend AnimationSystem;

    public:
        BoneBuffer(BoneRegistry* boneRegistry);
        BoneBuffer& operator=(const BoneBuffer&) = delete;

        ~BoneBuffer();

        void clear();

        void shutdown();
        void prepare();

    protected:
        void updateRT();

    private:
        void updateBuffer();

        void createBuffer(size_t totalCount);

        bool updateSpan(
            const std::vector<BoneTransformSSBO>& m_snapshot,
            size_t updateIndex,
            size_t updateCount);

    private:
        BoneRegistry* const m_boneRegistry;

        std::unique_ptr<kigl::GLSyncQueue<BoneTransformSSBO>> m_queue;

        size_t m_frameSkipCount{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };
    };
}
