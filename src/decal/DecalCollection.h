#pragma once

#include <vector>
#include <mutex>
#include <atomic>
#include <memory>

#include "Decal.h"
#include "DecalSSBO.h"

struct PrepareContext;
struct UpdateContext;

namespace decal {
    class DecalBuffer;

    class DecalCollection final
    {
        friend DecalBuffer;

    public:
        DecalCollection();
        ~DecalCollection();

        DecalCollection(DecalCollection&&);
        //DecalCollection& operator=(DecalCollection&& o);

        void bind() const;

        void clear();
        void prepare();

        void updateWT(const UpdateContext& ctx);
        void updateRT(const UpdateContext& ctx);

        void addDecal(const Decal& decal);

        uint32_t getActiveDecalCount() const noexcept {
            return static_cast<uint32_t>(m_activeCount);
        }

        bool isFull() const noexcept {
            return m_decals.size() >= m_maxCount;
        }

        bool isStatic() const noexcept {
            return m_static;
        }

        void setStatic(bool isStatic)
        {
            m_static = isStatic;
        }

    private:
        void snapshotDecals();

    private:
        bool m_static{ true };

        mutable std::unique_ptr<DecalBuffer> m_decalBuffer;

        std::mutex m_lock{};
        std::mutex m_snapshotLock{};

        std::atomic_bool m_updateReady{ false };

        bool m_dirty{ false };

        size_t m_maxCount{ 0 };
        std::vector<Decal> m_decals;

        std::vector<DecalSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };
    };
}
