#include "LightLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "component/Light.h"

namespace loader{
    LightLoader::LightLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void LightLoader::loadLight(
        const YAML::Node& node,
        LightData& data) const
    {
        // Default to center
        data.targetIdBase = { ROOT_UUID };

        data.enabled = true;

        // pos relative to owning node
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
                    data.type = LightType::none;
                }
                else if (type == "directional") {
                    data.type = LightType::directional;
                }
                else if (type == "point") {
                    data.type = LightType::point;
                }
                else if (type == "spot") {
                    data.type = LightType::spot;
                }
                else {
                    reportUnknown("light_type", k, v);
                }
            }
            else if (k == "pos") {
                throw std::runtime_error{ fmt::format("POS obsolete: {}", renderNode(node)) };
            }
            else if (k == "target_id") {
                data.targetIdBase = readUUID(v);
            }
            else if (k == "linear") {
                data.linear = readFloat(v);
            }
            else if (k == "quadratic") {
                data.quadratic = readFloat(v);
            }
            else if (k == "cutoff_angle") {
                data.cutoffAngle = readFloat(v);
            }
            else if (k == "outer_cutoff_angle") {
                data.outerCutoffAngle = readFloat(v);
            }
            else if (k == "diffuse") {
                data.diffuse = readRGB(v);
            }
            else if (k == "intensity") {
                data.intensity = readFloat(v);
            }
            else {
                reportUnknown("light_entry", k, v);
            }
        }
    }

    std::unique_ptr<Light> LightLoader::createLight(
        const LightData& data,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        if (!data.enabled) return std::unique_ptr<Light>();

        auto light = std::make_unique<Light>();

        light->m_enabled = true;
        light->setTargetId(resolveUUID(data.targetIdBase, cloneIndex, tile));

        light->linear = data.linear;
        light->quadratic = data.quadratic;

        light->cutoffAngle = data.cutoffAngle;
        light->outerCutoffAngle = data.outerCutoffAngle;

        light->diffuse = data.diffuse;
        light->intensity = data.intensity;

        switch (data.type) {
        case LightType::directional:
            light->m_directional = true;
            break;
        case LightType::point:
            light->m_point = true;
            break;
        case LightType::spot:
            light->m_spot = true;
            break;
        }

        return light;
    }

}
