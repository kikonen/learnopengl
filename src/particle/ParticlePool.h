#pragma once

#include <string>
#include <vector>
#include <mutex>

namespace render
{
    class RenderContext;
}

struct PrepareContext;
struct UpdateContext;

namespace particle
{
    struct Particle;
    struct ParticleSSBO;

    class ParticlePool
    {
        friend class ParticleSystem;

    public:
        ParticlePool(const std::string& name);
        ~ParticlePool();

        void clear();

        void prepare();

        void updateWT(const UpdateContext& ctx);

        bool isFull() const noexcept;

        uint32_t getFreespace() const noexcept;

        uint32_t getActiveParticleCount() const noexcept
        {
            return static_cast<uint32_t>(m_activeCount);
        }

        bool addParticle(const Particle& particle);

        uint32_t getBaseIndex() const noexcept
        {
            return m_baseIndex;
        }

    private:
        void preparePending();
        void snapshotParticles();
        void upload(ParticleSSBO* mappedData);

    private:
        mutable std::mutex m_pendingLock{};

        uint32_t m_baseIndex{ 0 };

        std::atomic_bool m_updateReady{ false };

        size_t m_maxCount{ 0 };
        std::vector<Particle> m_particles;

        std::vector<Particle> m_pending;

        std::vector<ParticleSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };
    };
}
