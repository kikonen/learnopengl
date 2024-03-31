#include "GeneratorLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "asset/Assets.h"
#include "asset/Material.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MaterialSet.h"

#include "generator/GridGenerator.h"
#include "generator/AsteroidBeltGenerator.h"
#include "terrain/TerrainGenerator.h"

namespace loader {
    GeneratorLoader::GeneratorLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void GeneratorLoader::loadGenerator(
        const YAML::Node& node,
        GeneratorData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                data.enabled = false;
            }
            else if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = GeneratorType::none;
                }
                else if (type == "terrain") {
                    data.type = GeneratorType::terrain;
                }
                else if (type == "grid") {
                    data.type = GeneratorType::grid;
                }
                else if (type == "asteroid_belt") {
                    data.type = GeneratorType::asteroid_belt;
                }
                else {
                    reportUnknown("generator_type", k, v);
                }
            }
            else if (k == "count") {
                data.count = readInt(v);
            }
            else if (k == "radius") {
                data.radius = readFloat(v);
            }
            else if (k == "mode") {
                data.mode = readInt(v);
            }
            else if (k == "repeat") {
                loadRepeat(v, data.repeat);
            }
            else if (k == "tiling") {
                loadTiling(v, data.tiling);
            }
            else if (k == "material") {
                data.materialName = readString(v);
            }
            else if (k == "terrain") {
                loadTerrain(v, data.terrainData);
            }
            else {
                reportUnknown("generator_entry", k, v);
            }
        }
    }

    void GeneratorLoader::loadTerrain(
        const YAML::Node& node,
        TerrainData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
            }
            else if (k == "map_height") {
                std::string line = readString(v);
                data.map_height = resolveTexturePath(line, false);
            }
            else {
                reportUnknown("terrain_entry", k, v);
            }
        }
    }

    std::unique_ptr<NodeGenerator> GeneratorLoader::createGenerator(
        const GeneratorData& data,
        const std::vector<MaterialData>& materials,
        mesh::MeshType* type)
    {
        if (!data.enabled) return nullptr;

        const auto& assets = Assets::get();

        switch (data.type) {
        case GeneratorType::terrain: {
            auto generator{ std::make_unique<terrain::TerrainGenerator>() };

            const auto& terrainData = data.terrainData;
            const auto& tiling = data.tiling;

            generator->m_modelsDir = assets.modelsDir;
            generator->m_worldTileSize = tiling.tile_size;
            generator->m_worldTilesU = tiling.tiles.x;
            generator->m_worldTilesV = tiling.tiles.z;
            generator->m_heightMapFile = terrainData.map_height;
            generator->m_verticalRange = tiling.vertical_range;
            generator->m_horizontalScale = tiling.horizontal_scale;

            auto* material = findMaterial(data.materialName, materials);
            if (material) {
                generator->m_material = *material;
                generator->m_material.loadTextures();
            }

            return generator;
        }
        case GeneratorType::asteroid_belt: {
            auto generator{ std::make_unique<AsteroidBeltGenerator>(data.count) };
            return generator;
        }
        case GeneratorType::grid: {
            auto generator{ std::make_unique<GridGenerator>() };
            generator->m_xCount = data.repeat.xCount;
            generator->m_yCount = data.repeat.yCount;
            generator->m_zCount = data.repeat.zCount;

            generator->m_xStep = data.repeat.xStep;
            generator->m_yStep = data.repeat.yStep;
            generator->m_zStep = data.repeat.zStep;

            return generator;
        }
        }

        return nullptr;
    }

}
