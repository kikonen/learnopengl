#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

#include "util/BufferReference.h"

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
        void prepare();

        void beginFrame();
        void endFrame();

    protected:
        void updateRT();

    private:
        void upload();
        void uploadSpan(
            const std::vector<SocketTransformSSBO>& snapshot,
            const util::BufferReference& range);

        void resizeBuffer(size_t totalCount);

    private:
        SocketRegistry* const m_socketRegistry;

        kigl::GLBuffer m_ssbo{ "socket_ssbo" };
        kigl::GLFence m_fence{ "socket_fence" };
        size_t m_entryCount{ 0 };

        size_t m_frameSkipCount{ 0 };
    };
}
