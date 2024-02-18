#pragma once

#include <vector>
#include <mutex>

#include "pool/TypeHandle.h"

#include "Particle.h"
#include "ParticleSSBO.h"

#include "kigl/GLBuffer.h"

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class Registry;
class Program;

namespace particle {
    class ParticleSystem final
    {
    public:
        static ParticleSystem& get() noexcept;

        ParticleSystem();

        void prepare();

        void updateWT(const UpdateContext& ctx);

        void updateRT(const UpdateContext& ctx);

        void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        bool isEnabled() { return m_enabled; }

        void addParticle(const Particle& particle);

        uint32_t getActiveParticleCount() const noexcept {
            return static_cast<uint32_t>(m_activeCount);
        }

        bool isFull() const noexcept {
            return !m_enabled || m_particles.size() >= m_maxCount;
        }

    private:
        void snapshotParticles();
        void updateParticleBuffer();

    private:
        bool m_enabled{ false };

        std::mutex m_lock{};
        std::mutex m_snapshotLock{};

        size_t m_maxCount{ 0 };
        std::vector<Particle> m_particles;

        pool::TypeHandle m_typeHandle{};

        std::vector<ParticleSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };

        kigl::GLBuffer m_ssbo{ "particle_ssbo" };
        size_t m_lastParticleSize{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}
