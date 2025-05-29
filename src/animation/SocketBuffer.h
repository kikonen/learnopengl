#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <span>

#include "kigl/GLBuffer.h"

namespace animation {
    class AnimationSystem;
    class SocketRegistry;

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

        bool updateSpan(
            const std::vector<glm::mat4>& snapshot,
            size_t updateIndex,
            size_t updateCount);

    private:
        SocketRegistry* const m_socketRegistry;

        kigl::GLBuffer m_ssbo{ "bone_socket_ssbo" };

        size_t m_frameSkipCount{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };
    };
}
