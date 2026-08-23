#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#include "kigl/GLBuffer.h"
#include "kigl/GLFence.h"

namespace render
{
    class RenderContext;
}

struct PrepareContext;
struct UpdateContext;

namespace particle {
    class ParticlePool;
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

        void beginFrame();
        void endFrame();

        void updateWT(const UpdateContext& ctx);
        void updateRT(const UpdateContext& ctx);

        void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        bool isEnabled() const noexcept { return m_enabled; }

        uint32_t getPoolCount() const noexcept
        {
            return 2;
        }

        ParticlePool* getPool(uint32_t poolIndex);

        uint32_t getActiveParticleCount() const noexcept;

    private:
        void upload(size_t poolIndex);
        bool resizeBuffer(size_t maxCount);

    private:
        bool m_enabled{ false };

        size_t m_updatePoolIndexWT{ 0 };
        size_t m_updatePoolIndexRT{ 0 };

        std::vector<std::unique_ptr<ParticlePool>> m_pools;
        std::vector<std::unique_ptr<std::mutex>> m_snapshotLocks;

        size_t m_frameSkipCount{ 0 };

        kigl::GLBuffer m_ssbo{ "particle_ssbo" };
        kigl::GLFence m_fence{ "particle_fence" };
        size_t m_entryCount{ 0 };
    };
}
