#pragma once

#include <mutex>
#include <vector>

#include "kigl/GLBuffer.h"

#include "TerrainTileInfo.h"
#include "TerrainTileSSBO.h"

struct UpdateContext;


namespace terrain {
    class TerrainTileRegistry {
    public:
        static void init() noexcept;
        static void release() noexcept;
        static TerrainTileRegistry& get() noexcept;

        TerrainTileRegistry();
        ~TerrainTileRegistry();

        void clear();
        void prepare();

        void updateWT(const UpdateContext& ctx);
        void updateRT(const UpdateContext& ctx);

        uint32_t addTile(TerrainTileInfo& tile);

    private:
        void snapshotTiles();
        void updateTileBuffer();

    private:
        bool m_dirty{ false };

        std::mutex m_lock{};
        std::mutex m_snapshotLock{};

        std::atomic_bool m_updateReady{ false };

        std::vector<TerrainTileInfo> m_tiles;

        std::vector<TerrainTileSSBO> m_snapshot;
        size_t m_snapshotCount{ 0 };
        size_t m_activeCount{ 0 };

        kigl::GLBuffer m_ssbo{ "terrain_tile_ssbo" };
        size_t m_lastTilesSize{ 0 };

        bool m_useMapped{ false };
        bool m_useInvalidate{ false };
        bool m_useFence{ false };
        bool m_useFenceDebug{ false };
    };
}
