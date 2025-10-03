#include "DecalBuffer.h"

#include "asset/Assets.h"

#include "util/thread.h"

#include "kigl/GLState.h"

#include "shader/SSBO.h"
#include "shader/ProgramRegistry.h"

#include "engine/UpdateContext.h"
#include "engine/PrepareContext.h"

#include "render/RenderContext.h"
#include "debug/DebugContext.h"

#include "registry/Registry.h"

#include "Decal.h"
#include "DecalCollection.h"

namespace {
    constexpr size_t BLOCK_SIZE = 10000;
    constexpr size_t MAX_BLOCK_COUNT = 1100;
}

namespace decal {
    DecalBuffer::DecalBuffer(DecalCollection* collection)
        : m_collection{ collection }
    {
    }

    DecalBuffer::~DecalBuffer() = default;

    void DecalBuffer::clear()
    {
        ASSERT_RT();

        m_lastDecalSize = 0;
    }

    void DecalBuffer::shutdown() {
        ASSERT_RT();

        clear();
    }

    void DecalBuffer::prepare() {
        ASSERT_RT();

        const auto& assets = Assets::get();

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useFenceDebug = assets.glUseFenceDebug;

        m_useMapped = false;
        m_useInvalidate = true;
        m_useFence = false;
        m_useFenceDebug = false;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(DecalSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_DECALS);

        clear();
    }

    void DecalBuffer::bind()
    {
        m_ssbo.bindSSBO(SSBO_DECALS);
    }

    void DecalBuffer::update(const UpdateContext& ctx)
    {
        if (!m_collection->m_updateReady) return;
        if (!ctx.getDebug().m_decalEnabled) return;

        //m_frameSkipCount++;
        //if (m_frameSkipCount < 2) {
        //    return;
        //}
        //m_frameSkipCount = 0;

        updateBuffer(m_collection->m_snapshot);
    }

    void DecalBuffer::updateBuffer(
        const std::vector<DecalSSBO>& snapshot)
    {
        std::lock_guard lock(m_collection->m_snapshotLock);

        if (m_collection->m_snapshotCount == 0) {
            m_collection->m_activeCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(DecalSSBO);
        const size_t totalCount = m_collection->m_snapshotCount;

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            if (m_ssbo.resizeBuffer(bufferSize, false)) {
                m_ssbo.bindSSBO(SSBO_DECALS);
            }
        }

        //m_ssbo.invalidateRange(
        //    0,
        //    totalCount * sz);

        //if (m_useInvalidate) {
        //    m_ssbo.invalidateRange(0, totalCount * sz);
        //}

        m_ssbo.update(
            0,
            totalCount * sz,
            snapshot.data());

        m_ssbo.markUsed(totalCount * sz);

        m_collection->m_activeCount = totalCount;

        m_collection->m_updateReady = false;
    }
}
