#include "LightLoader.h"

#include "util/util.h"

#include "component/Light.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader{
    LightLoader::LightLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void LightLoader::loadLight(
        const loader::DocNode& node,
        LightData& data) const
    {
        // Default to center
        data.targetBaseId = { ROOT_ID };

        data.enabled = true;

        // pos relative to owning node
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
                data.targetBaseId = readId(v);
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
            else if (k == "diffuse" || k == "kd") {
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

        auto [targetId, targetResolvedSID] = resolveId(data.targetBaseId, cloneIndex, tile, false);
        light->setTargetId(targetId);

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
