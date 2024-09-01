#pragma once

#include "generator/NodeGenerator.h"

#include "material/Material.h"
#include "material/Image.h"

#include "physics/size.h"

#include "TerrainTileInfo.h"

class ImageTexture;

namespace mesh {
    class ModelMesh;
}

class Registry;

namespace terrain {
    //
    // Tessellated terrain generator
    //
    // NOTE KI tessellated terraiin *CANNOT* currently produce shadows
    // - would require separate special terrain-shadow renderer
    // - also tessellation level likely *would* pose problems with jumping terrain
    //   * distance from light is different than from camera
    class TerrainGenerator final : public NodeGenerator
    {
    public:
        TerrainGenerator();

        virtual void prepareWT(
            const PrepareContext& ctx,
            Node& container) override;

        virtual void updateWT(
            const UpdateContext& ctx,
            const Node& container) override;

    private:
        void updateTiles(
            const UpdateContext& ctx,
            const Node& container);

        physics::height_map_id prepareHeightMap(
            const PrepareContext& ctx,
            const Node& container);

        ImageTexture* loadTexture(bool flipY);

        void createTiles(
            const PrepareContext& ctx,
            const Node& container,
            physics::height_map_id heightMapId);

        pool::TypeHandle createType(
            Registry* registry,
            pool::TypeHandle containerTypeHandle);

    public:
        int m_worldTileSize{ 100 };
        int m_worldTilesU{ 1 };
        int m_worldTilesV{ 1 };

        glm::vec2 m_verticalRange{ 0.f, 32.f };
        float m_horizontalScale{ 1.f };

        std::string m_heightMapFile;
        ImageTexture* m_heightMapTex{ nullptr };

        Material m_material;

        std::string m_modelsDir;

    private:
        size_t m_gridSize{ 0 };

        size_t m_poolSizeU{ 0 };
        size_t m_poolSizeV{ 0 };

        pool::NodeHandle m_nodeHandle{};

        std::vector<TerrainTileInfo> m_tileInfos;
    };
}
