#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#include "Decal.h"
#include "DecalSSBO.h"

#include "kigl/GLBuffer.h"

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class Registry;
class Program;

namespace decal {
    class DecalSystem final
    {
    public:
        static DecalSystem& get() noexcept;

        DecalSystem();

        void prepare();

        void updateWT(const UpdateContext& ctx);

        void updateRT(const UpdateContext& ctx);

        void setEnabled(bool enabled) {
            m_enabled = enabled;
        }

        bool isEnabled() const noexcept { return m_enabled; }

        void addDecal(const Decal& decal);

        uint32_t getActiveDecalCount() const noexcept {
            return static_cast<uint32_t>(m_activeCount);
        }

        bool isFull() const noexcept {
            return !m_enabled || m_decals.size() >= m_maxCount;
        }

    private:
        void snapshotDecals();
        void updateDecalBuffer();

    private:
        bool m_enabled{ false };

        std::mutex m_lock{};
        std::mutex m_snapshotLock{};

        std::atomic_bool m_updateReady{ false };
        size_t m_frameSkipCount{ 0 };

        size_t m_maxCount{ 0 };
        std::vector<Decal> m_decals;

        std::vector<DecalSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };

        kigl::GLBuffer m_ssbo{ "decal_ssbo" };
        size_t m_lastDecalSize{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useDebugFence{ false };
    };
}
