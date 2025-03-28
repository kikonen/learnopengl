#include "GeneratorLoader.h"

#include "util/util.h"

#include "asset/Assets.h"
#include "material/Material.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"
#include "mesh/MeshFlags.h"

#include "generator/GridGenerator.h"
#include "generator/AsteroidBeltGenerator.h"
#include "terrain/TerrainGenerator.h"

#include "physics/Category.h"
#include "physics/physics_util.h"

#include "loader/document.h"
#include "loader_util.h"

#include "loader/Loaders.h"
#include "value/PhysicsCategoryValue.h"
#include "value/PhysicsGeomValue.h"

namespace {
    std::unordered_map<std::string, GeneratorMode> g_modeMapping;

    const std::unordered_map<std::string, GeneratorMode>& getModeMapping()
    {
        if (g_modeMapping.empty()) {
            g_modeMapping.insert({
                { "none", GeneratorMode::none },
                { "grid", GeneratorMode::grid },
                { "random", GeneratorMode::random },
                });
        }
        return g_modeMapping;
    }

    GeneratorMode readMode(std::string v)
    {
        {
            const auto& mapping = getModeMapping();
            const auto& it = mapping.find(v);
            if (it != mapping.end()) return it->second;
        }

        // NOTE KI for data tracking data mismatches
        KI_WARN_OUT(fmt::format("GENERATOR: INVALID_MODE- mode={}", v));
        return GeneratorMode::none;
    }
}

namespace loader {
    GeneratorLoader::GeneratorLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void GeneratorLoader::loadGenerator(
        const loader::DocNode& node,
        GeneratorData& data,
        Loaders& loaders) const
    {
        data.enabled = true;

        data.boundsMask = physics::mask(physics::Category::terrain);

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

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
                data.mode = readMode(readString(v));
            }
            else if (k == "offset") {
                data.offset = readVec3(v);
            }
            else if (k == "scale") {
                data.scale = readFloat(v);
            }
            else if (k == "seed") {
                data.seed = readUVec3(v);
            }
            else if (k == "repeat") {
                loadRepeat(v, data.repeat);
            }
            else if (k == "tiling") {
                loadTiling(v, data.tiling);
            }
            else if (k == "bounds_dir") {
                data.boundsDir = readVec3(v);
            }
            else if (k == "bounds") {
                PhysicsCategoryValue loader;
                loader.loadMask(v, data.boundsMask);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(v, data.materialData, loaders);
            }
            else if (k == "geom") {
                PhysicsGeomValue loader;
                loader.loadGeom(v, data.geom);
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
        const loader::DocNode& node,
        TerrainData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "enabled") {
            }
            else if (k == "map_height") {
                std::string line = readString(v);
                data.map_height = line;
            }
            else {
                reportUnknown("terrain_entry", k, v);
            }
        }
    }

    std::unique_ptr<NodeGenerator> GeneratorLoader::createGenerator(
        const GeneratorData& data,
        mesh::MeshType* type,
        Loaders& loaders)
    {
        if (!data.enabled) return nullptr;

        const auto& assets = Assets::get();

        switch (data.type) {
        case GeneratorType::terrain: {
            auto generator{ std::make_unique<terrain::TerrainGenerator>() };

            const auto& terrainData = data.terrainData;
            const auto& tiling = data.tiling;

            generator->m_mode = data.mode;
            generator->m_offset = data.offset;
            generator->m_scale = data.scale;

            generator->m_modelsDir = assets.modelsDir;
            generator->m_worldTileSize = tiling.tile_size;
            generator->m_worldTilesU = tiling.tiles.x;
            generator->m_worldTilesV = tiling.tiles.z;
            generator->m_heightMapFile = terrainData.map_height;
            generator->m_verticalRange = tiling.vertical_range;
            generator->m_horizontalScale = tiling.horizontal_scale;

            generator->m_material = data.materialData.material;
            generator->m_material.loadTextures();

            loaders.m_materialLoader.resolveMaterial({}, generator->m_material);

            return generator;
        }
        case GeneratorType::asteroid_belt: {
            auto generator{ std::make_unique<AsteroidBeltGenerator>(data.count) };

            generator->m_mode = data.mode;
            generator->m_offset = data.offset;
            generator->m_scale = data.scale;

            return generator;
        }
        case GeneratorType::grid: {
            auto generator{ std::make_unique<GridGenerator>() };

            generator->m_mode = data.mode;
            generator->m_offset = data.offset;
            generator->m_scale = data.scale;

            generator->m_seed = data.seed;

            generator->m_boundsDir = glm::normalize(data.boundsDir);
            generator->m_boundsMask = data.boundsMask;

            generator->m_count = data.count;
            generator->m_xCount = data.repeat.xCount;
            generator->m_yCount = data.repeat.yCount;
            generator->m_zCount = data.repeat.zCount;

            generator->m_xStep = data.repeat.xStep;
            generator->m_yStep = data.repeat.yStep;
            generator->m_zStep = data.repeat.zStep;

            generator->m_geometryTemplate = data.geom;

            return generator;
        }
        }

        return nullptr;
    }

}
