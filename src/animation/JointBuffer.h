#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>
#include <memory>

#include "kigl/GLSyncQueue.h"

namespace animation {
    class AnimationSystem;
    class JointRegistry;

    struct JointTransformSSBO;

    class JointBuffer {
        friend AnimationSystem;

    public:
        JointBuffer(JointRegistry* jointRegistry);
        JointBuffer& operator=(const JointBuffer&) = delete;

        ~JointBuffer();

        void clear();
        void prepare();

    protected:
        void updateRT();

    private:
        void updateBuffer();

        void createBuffer(size_t totalCount);

        bool updateSpan(
            const std::vector<JointTransformSSBO>& m_snapshot,
            size_t updateIndex,
            size_t updateCount);

    private:
        JointRegistry* const m_jointRegistry;

        std::unique_ptr<kigl::GLSyncQueue<JointTransformSSBO>> m_queue;

        size_t m_frameSkipCount{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };
    };
}
