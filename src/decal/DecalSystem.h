#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#include "Decal.h"
#include "DecalSSBO.h"

struct PrepareContext;
struct UpdateContext;
class RenderContext;

class Registry;
class Program;

namespace decal {
    class DecalBuffer;

    class DecalSystem final
    {
        friend DecalBuffer;

    public:
        static DecalSystem& get() noexcept;

        DecalSystem();
        ~DecalSystem();

        void clearWT();
        void shutdownWT();
        void prepareWT();

        void updateWT(const UpdateContext& ctx);

        void clearRT();
        void shutdownRT();
        void prepareRT();

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

    private:
        bool m_enabled{ false };

        std::unique_ptr<DecalBuffer> m_decalBuffer;

        std::mutex m_lock{};
        std::mutex m_snapshotLock{};

        std::atomic_bool m_updateReady{ false };

        size_t m_maxCount{ 0 };
        std::vector<Decal> m_decals;

        std::vector<DecalSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };
    };
}
