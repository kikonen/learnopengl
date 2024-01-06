#include "GeneratorLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "model/Node.h"

#include "mesh/MeshType.h"
#include "mesh/MaterialVBO.h"

#include "generator/GridGenerator.h"
#include "generator/TerrainGenerator.h"
#include "generator/AsteroidBeltGenerator.h"


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
            else {
                reportUnknown("generator_entry", k, v);
            }
        }
    }

    std::unique_ptr<NodeGenerator> GeneratorLoader::createGenerator(
        const GeneratorData& data,
        Node* node)
    {
        if (!data.enabled) return nullptr;

        const auto& center = node->getTransform().getPosition();

        switch (data.type) {
        case GeneratorType::terrain: {
            auto generator{ std::make_unique<TerrainGenerator>() };

            auto& materialVBO = node->m_type->m_materialVBO;
            const auto& tiling = data.tiling;

            generator->m_modelsDir = m_assets.modelsDir;
            generator->m_worldTileSize = tiling.tile_size;
            generator->m_worldTilesU = tiling.tiles.x;
            generator->m_worldTilesV = tiling.tiles.z;
            generator->m_verticalRange = tiling.vertical_range;
            generator->m_horizontalScale = tiling.horizontal_scale;

            auto* material = materialVBO->getDefaultMaterial();
            if (material) {
                generator->m_material = *material;
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
