#include "TerrainTileRegistry.h"

#include "asset/Assets.h"

#include "material/ImageTexture.h"

#include "shader/SSBO.h"

namespace {
    constexpr size_t BLOCK_SIZE = 16;
    constexpr size_t MAX_BLOCK_COUNT = 100;

    static terrain::TerrainTileRegistry* s_registry{ nullptr };
}

namespace terrain {
    void TerrainTileRegistry::init() noexcept
    {
        assert(!s_registry);
        s_registry = new TerrainTileRegistry();
    }

    void TerrainTileRegistry::release() noexcept
    {
        auto* s = s_registry;
        s_registry = nullptr;
        delete s;
    }

    TerrainTileRegistry& TerrainTileRegistry::get() noexcept
    {
        assert(s_registry);
        return *s_registry;
    }
}

namespace terrain {
    TerrainTileRegistry::TerrainTileRegistry()
    {
        // NOTE KI null entry
        TerrainTileInfo info;
        addTile(info);
    }

    TerrainTileRegistry::~TerrainTileRegistry() = default;

    void TerrainTileRegistry::prepare()
    {
        const auto& assets = Assets::get();

        m_useMapped = assets.glUseMapped;
        m_useInvalidate = assets.glUseInvalidate;
        m_useFence = assets.glUseFence;
        m_useFenceDebug = assets.glUseFenceDebug;

        m_useMapped = false;
        m_useInvalidate = true;
        m_useFence = false;
        //m_useFenceDebug = false;

        m_ssbo.createEmpty(1 * BLOCK_SIZE * sizeof(TerrainTileSSBO), GL_DYNAMIC_STORAGE_BIT);
        m_ssbo.bindSSBO(SSBO_TERRAIN_TILES);
    }

    void TerrainTileRegistry::clearWT()
    {
        std::lock_guard lock(m_lock);
    }

    void TerrainTileRegistry::updateWT(const UpdateContext& ctx)
    {
        std::lock_guard lock(m_lock);

        snapshotTiles();
    }

    void TerrainTileRegistry::clearRT()
    {
        std::lock_guard lock(m_lock);
    }

    void TerrainTileRegistry::updateRT(const UpdateContext& ctx)
    {
        if (!m_updateReady) return;

        {
            std::lock_guard lock(m_lock);
            for (size_t i = 0; i < m_tiles.size(); i++) {
                auto& tile = m_tiles[i];

                if (!tile.m_heightMapTex) continue;
                if (tile.m_heightMapTexHandle) continue;

                tile.m_heightMapTex->prepare();
                tile.m_heightMapTexHandle = tile.m_heightMapTex->m_handle;
                m_snapshot[i].u_heightMapTex = tile.m_heightMapTexHandle;
            }
        }

        updateTileBuffer();
    }

    uint32_t TerrainTileRegistry::addTile(TerrainTileInfo& tile)
    {
        std::lock_guard lock(m_lock);

        tile.m_registeredIndex = static_cast<uint32_t>(m_tiles.size());

        m_tiles.push_back(tile);
        m_dirty = true;

        return tile.m_registeredIndex;
    }

    void TerrainTileRegistry::snapshotTiles()
    {
        if (!m_dirty) return;

        std::lock_guard lock(m_snapshotLock);

        if (m_tiles.empty()) {
            m_snapshotCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(TerrainTileSSBO);
        const size_t totalCount = m_tiles.size();

        if (m_snapshotCount != totalCount) {
            m_snapshot.resize(totalCount);
        }

        for (size_t i = 0; i < totalCount; i++) {
            m_tiles[i].updateSSBO(m_snapshot[i]);
        }

        m_snapshotCount = totalCount;
        m_updateReady = true;

        m_dirty = false;
    }

    void TerrainTileRegistry::updateTileBuffer()
    {
        std::lock_guard lock(m_snapshotLock);

        if (m_snapshotCount == 0) {
            m_activeCount = 0;
            return;
        }

        constexpr size_t sz = sizeof(TerrainTileSSBO);
        const size_t totalCount = m_snapshotCount;

        if (m_ssbo.m_size < totalCount * sz) {
            size_t blocks = (totalCount / BLOCK_SIZE) + 2;
            size_t bufferSize = blocks * BLOCK_SIZE * sz;
            m_ssbo.resizeBuffer(bufferSize, false);
            m_ssbo.bindSSBO(SSBO_TERRAIN_TILES);
        }

        //m_ssbo.invalidateRange(
        //    0,
        //    totalCount * sz);

        m_ssbo.update(
            0,
            totalCount * sz,
            m_snapshot.data());

        m_ssbo.markUsed(totalCount * sz);

        m_activeCount = totalCount;

        m_updateReady = false;
    }
}
