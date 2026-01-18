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
    const model::NodeType* type)
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

        if (data.m_shape.m_enabled)
        {
            auto& shapeData = data.m_shape;
            generator->m_shapeTemplate = std::make_unique<ShapeDefinition>();
            auto& shape = *generator->m_shapeTemplate;

            shape.m_size = shapeData.m_size;

            shape.m_rotation = shapeData.m_rotation;
            shape.m_offset = shapeData.m_offset;

            shape.m_category = shapeData.m_category;
            shape.m_collisionMask = shapeData.m_collisionMask;

            shape.m_type = shapeData.m_type;

            shape.m_placeable = shapeData.m_placeable;
        }

        return generator;
    }
    }

    return nullptr;
}
