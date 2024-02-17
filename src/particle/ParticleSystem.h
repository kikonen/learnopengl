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

        void addParticle(const Particle& particle);

        uint32_t getActiveParticleCount() const noexcept {
            return static_cast<uint32_t>(m_activeCount);
        }

    private:
        void updateParticleBuffer();

    private:
        std::mutex m_lock{};

        std::vector<Particle> m_particles;

        pool::TypeHandle m_typeHandle{};

        std::vector<ParticleSSBO> m_entries;
        size_t m_activeCount{ 0 };

        kigl::GLBuffer m_ssbo{ "particle_ssbo" };
        size_t m_lastParticleSize{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}
