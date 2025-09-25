#pragma once

#include <vector>
#include <mutex>
#include <atomic>

#include "kigl/GLSyncQueue.h"

namespace render
{
    class RenderContext;
}

struct PrepareContext;
struct UpdateContext;

class Registry;
class Program;

namespace particle {
    struct Particle;
    struct ParticleSSBO;

    class ParticleSystem final
    {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static ParticleSystem& get() noexcept;

        ParticleSystem();
        ~ParticleSystem();

        void clear();

        void prepare();

        void updateWT(const UpdateContext& ctx);

        void updateRT(const UpdateContext& ctx);

        void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        bool isEnabled() const noexcept { return m_enabled; }

        // @return true if was added, false if full
        bool addParticle(const Particle& particle);

        uint32_t getActiveParticleCount() const noexcept
        {
            return static_cast<uint32_t>(m_activeCount);
        }

        uint32_t getFreespace() const noexcept;

        bool isFull() const noexcept;

    private:

        void preparePending();

        void snapshotParticles();
        void updateParticleBuffer();
        void createParticleBuffer();

    private:
        bool m_enabled{ false };

        mutable std::mutex m_pendingLock{};
        std::mutex m_snapshotLock{};

        std::atomic_bool m_updateReady{ false };
        size_t m_frameSkipCount{ 0 };

        size_t m_maxCount{ 0 };
        std::vector<Particle> m_particles;

        std::vector<Particle> m_pending;

        std::vector<ParticleSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };

        std::unique_ptr<kigl::GLSyncQueue<ParticleSSBO>> m_queue;
        size_t m_lastParticleSize{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };
    };
}
