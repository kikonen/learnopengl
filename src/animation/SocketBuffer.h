#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLSyncQueue.h"

namespace animation {
    class AnimationSystem;
    class SocketRegistry;

    struct SocketTransformSSBO;

    class SocketBuffer {
        friend AnimationSystem;

    public:
        SocketBuffer(SocketRegistry* socketRegistry);
        SocketBuffer& operator=(const SocketBuffer&) = delete;

        ~SocketBuffer();

        void clear();

        void shutdown();
        void prepare();

    protected:
        void updateRT();

    private:
        void updateBuffer();
        void createBuffer(size_t totalCount);

        bool updateSpan(
            const std::vector<SocketTransformSSBO>& snapshot,
            size_t updateIndex,
            size_t updateCount);

    private:
        SocketRegistry* const m_socketRegistry;

        std::unique_ptr<kigl::GLSyncQueue<SocketTransformSSBO>> m_queue;

        size_t m_frameSkipCount{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };
    };
}
