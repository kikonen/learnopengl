#include "DecalCollection.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "shader/SSBO.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "debug/DebugContext.h"

#include "Decal.h"
#include "DecalBuffer.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;
}

namespace decal {
    DecalCollection::DecalCollection()
        : m_decalBuffer{ std::make_unique<DecalBuffer>(this) }
    {
    }

    DecalCollection::~DecalCollection() = default;

    DecalCollection::DecalCollection(DecalCollection&& o)
        : m_decalBuffer{ std::move(o.m_decalBuffer) },
        m_static{ o.m_static },
        m_dirty{ o.m_dirty },
        m_maxCount{ o.m_maxCount },
        m_decals{ o.m_decals },
        m_snapshot{ o.m_snapshot },
        m_snapshotCount{ o.m_snapshotCount },
        m_activeCount{ o.m_activeCount }
    { }

    //DecalCollection& DecalCollection::operator=(DecalCollection&& o)
    //{
    //    m_decalBuffer = std::move(o.m_decalBuffer);
    //    return *this;
    //}

    void DecalCollection::bind() const
    {
        m_decalBuffer->bind();
    }

    void DecalCollection::clearWT()
    {
        ASSERT_WT();

        m_updateReady = false;

        m_decals.clear();
        m_snapshot.clear();

        m_snapshotCount = 0;
        m_activeCount = 0;

        m_decals.reserve(1 * BLOCK_SIZE);
    }

    void DecalCollection::shutdownWT()
    {
        ASSERT_WT();

        clearWT();
    }

    void DecalCollection::prepareWT() {
        ASSERT_WT();

        const auto& assets = Assets::get();

        m_maxCount = std::min<int>(assets.decalMaxCount, MAX_BLOCK_COUNT * BLOCK_SIZE);

        clearWT();
    }

    void DecalCollection::clearRT()
    {
        ASSERT_RT();

        m_decalBuffer->clear();
    }

    void DecalCollection::shutdownRT()
    {
        ASSERT_RT();

        m_decalBuffer->shutdown();
    }

    void DecalCollection::prepareRT()
    {
        ASSERT_RT();

        m_decalBuffer->prepare();
    }

    void DecalCollection::addDecal(const Decal& decal)
    {
        if (!decal.m_parent) return;

        std::lock_guard lock(m_lock);
        if (isFull()) return;

        m_decals.push_back(decal);
        m_dirty = true;
    }

    void DecalCollection::updateWT(const UpdateContext& ctx)
    {
        if (!ctx.getDebug().m_decalEnabled) return;

        std::lock_guard lock(m_lock);

        bool dirty = m_dirty;

        if (!m_static) {
            size_t size = m_decals.size();
            for (size_t i = 0; i < size; i++) {
                auto& decal = m_decals[i];
                if (!decal.update(ctx)) {
                    if (i < size - 1) {
                        m_decals[i] = m_decals[size - 1];
                    }
                    size--;
                    i--;
                }
                dirty |= true;
            }

            if (size != m_decals.size()) {
                m_decals.resize(size);
            }
        }

        if (dirty) {
            snapshotDecals();
            m_dirty = false;
        }
    }

    void DecalCollection::updateRT(const UpdateContext& ctx)
    {
        m_decalBuffer->update(ctx);
    }

    void DecalCollection::snapshotDecals()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_decals.empty()) {
            m_snapshotCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(DecalSSBO);
        const size_t totalCount = m_decals.size();

        if (m_snapshotCount != totalCount) {
            m_snapshot.resize(totalCount);
        }

        for (size_t i = 0; i < totalCount; i++) {
            m_decals[i].updateSSBO(m_snapshot[i]);
        }

        m_snapshotCount = totalCount;
        m_updateReady = true;
    }
}
