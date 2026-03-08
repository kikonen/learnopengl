#include "GeneratorLoader.h"

#include "util/util.h"

#include "asset/Assets.h"
#include "material/Material.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshFlags.h"

#include "physics/Category.h"
#include "physics/physics_util.h"

#include "component/definition/PhysicsDefinition.h"
#include "component/definition/GeneratorDefinition.h"

#include "loader/document.h"
#include "loader_util.h"

#include "loader/Loaders.h"
#include "value/PhysicsCategoryValue.h"
#include "value/PhysicsShapeValue.h"

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
        const std::shared_ptr<Context>& ctx)
        : BaseLoader(ctx)
    {
    }

    void GeneratorLoader::loadGenerator(
        const loader::DocNode& node,
        const std::string& currentDir,
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
                loadTerrainTiling(v, data.tiling);
            }
            else if (k == "bounds_dir") {
                data.boundsDir = readVec3(v);
            }
            else if (k == "bounds") {
                PhysicsCategoryValue loader;
                loader.loadMask(v, data.boundsMask);
            }
            else if (k == "material") {
                loaders.m_materialLoader.loadMaterial(
                    v,
                    currentDir,
                    data.materialData,
                    loaders);
            }
            else if (k == "geom" || k == "shape") {
                PhysicsShapeValue loader;
                loader.loadShape(v, data.shape);
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

    std::unique_ptr<GeneratorDefinition> GeneratorLoader::createGeneratorDefinition(
        const GeneratorData& data,
        Loaders& loaders)
    {
        if (!data.enabled) return nullptr;

        const auto& assets = Assets::get();

        auto definition = std::make_unique<GeneratorDefinition>();
        auto& df = *definition;

        df.m_type = data.type;

        df.m_count = data.count;
        df.m_radius = data.radius;

        df.m_mode = data.mode;

        df.m_offset = data.offset;
        df.m_scale = data.scale;

        df.m_seed = data.seed;

        df.m_repeat = data.repeat;
        df.m_tiling = data.tiling;

        df.m_heightMap = data.terrainData.map_height;

        {
            auto& shapeData = data.shape;
            auto& shape = df.m_shape;

            shape.m_enabled = data.enabled;

            shape.m_size = shapeData.size;

            shape.m_baseAxis = shapeData.baseAxis;
            shape.m_baseFront = shapeData.baseFront;
            shape.m_baseAdjust = shapeData.baseAdjust;
            shape.m_offset = shapeData.offset;

            shape.m_category = shapeData.category;
            shape.m_collisionMask = shapeData.collisionMask;

            shape.m_type = shapeData.type;

            shape.m_placeable = shapeData.placeable;
        }

        df.m_boundsDir = data.boundsDir;
        df.m_boundsMask = data.boundsMask;

        {
            df.m_material = std::make_unique<Material>();
            *df.m_material = data.materialData.material;
            loaders.m_materialLoader.resolveMaterial({}, *df.m_material);
            df.m_material->loadTextures();
        }

        return definition;
    }
}
