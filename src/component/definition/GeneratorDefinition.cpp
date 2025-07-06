#include "GeneratorDefinition.h"

#include "asset/Assets.h"

#include "model/NodeType.h"

#include "generator/GridGenerator.h"
#include "generator/AsteroidBeltGenerator.h"
#include "terrain/TerrainGenerator.h"

#include "physics/Category.h"
#include "physics/physics_util.h"

#include "component/definition/GeneratorDefinition.h"

std::unique_ptr<NodeGenerator> GeneratorDefinition::createGenerator(
    const NodeType* type)
{
    if (!type->m_generatorDefinition) return nullptr;

    const auto& data = *type->m_generatorDefinition;

    const auto& assets = Assets::get();

    switch (data.m_type) {
    case GeneratorType::terrain: {
        auto generator{ std::make_unique<terrain::TerrainGenerator>() };

        const auto& tiling = data.m_tiling;

        generator->m_mode = data.m_mode;
        generator->m_offset = data.m_offset;
        generator->m_scale = data.m_scale;

        generator->m_modelsDir = assets.modelsDir;
        generator->m_worldTileSize = tiling.tile_size;
        generator->m_worldTilesU = tiling.tiles.x;
        generator->m_worldTilesV = tiling.tiles.z;
        generator->m_heightMapFile = data.m_heightMap;
        generator->m_verticalRange = tiling.vertical_range;
        generator->m_horizontalScale = tiling.horizontal_scale;

        generator->m_material = *data.m_material;

        return generator;
    }
    case GeneratorType::asteroid_belt: {
        auto generator{ std::make_unique<AsteroidBeltGenerator>(data.m_count) };

        generator->m_mode = data.m_mode;
        generator->m_offset = data.m_offset;
        generator->m_scale = data.m_scale;

        return generator;
    }
    case GeneratorType::grid: {
        auto generator{ std::make_unique<GridGenerator>() };

        generator->m_mode = data.m_mode;
        generator->m_offset = data.m_offset;
        generator->m_scale = data.m_scale;

        generator->m_seed = data.m_seed;

        generator->m_boundsDir = glm::normalize(data.m_boundsDir);
        generator->m_boundsMask = data.m_boundsMask;

        generator->m_count = data.m_count;

        const auto& repeat = data.m_repeat;

        generator->m_xCount = repeat.xCount;
        generator->m_yCount = repeat.yCount;
        generator->m_zCount = repeat.zCount;

        generator->m_xStep = repeat.xStep;
        generator->m_yStep = repeat.yStep;
        generator->m_zStep = repeat.zStep;

        if (data.m_geom.m_enabled)
        {
            auto& geomData = data.m_geom;
            generator->m_geometryTemplate = std::make_unique<GeomDefinition>();
            auto& geom = *generator->m_geometryTemplate;

            geom.m_size = geomData.m_size;

            geom.m_rotation = geomData.m_rotation;
            geom.m_offset = geomData.m_offset;

            geom.m_categoryMask = geomData.m_categoryMask;
            geom.m_collisionMask = geomData.m_collisionMask;

            geom.m_type = geomData.m_type;

            geom.m_placeable = geomData.m_placeable;
        }

        return generator;
    }
    }

    return nullptr;
}
