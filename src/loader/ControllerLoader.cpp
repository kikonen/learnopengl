#include "ControllerLoader.h"

#include "util/glm_util.h"
#include "util/util.h"

#include "model/Node.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    ControllerLoader::ControllerLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx)
    {
    }

    void ControllerLoader::loadControllers(
        const loader::DocNode& node,
        std::vector<ControllerData>& controllers) const
    {
        for (const auto& entry : node.getNodes()) {
            ControllerData& data = controllers.emplace_back();
            loadController(entry, data);
        }
    }

    void ControllerLoader::loadController(
        const loader::DocNode& node,
        ControllerData& data) const
    {
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
                    data.type = ControllerType::none;
                }
                else if (type == "pawn") {
                    data.type = ControllerType::pawn;
                }
                else if (type == "camera_zoom") {
                    data.type = ControllerType::camera_zoom;
                }
                else {
                    reportUnknown("controller_type", k, v);
                }
            }
            else if (k == "speed") {
                data.speed = readFloat(v);
            }
            else if (k == "mode") {
                data.mode = readInt(v);
            }
            else if (k == "target_id") {
                data.targetBaseId = readId(v);
            }
            else if (k == "dir") {
                data.direction = readVec3(v);
            }
            else if (k == "distance") {
                data.distance = readFloat(v);
            }
            else {
                reportUnknown("controller_entry", k, v);
            }
        }
    }

    std::unique_ptr<ControllerDefinition> ControllerLoader::createControllerDefinition(
        const ControllerData& data)
    {
        auto definition = std::make_unique<ControllerDefinition>();
        auto& df = *definition;

        auto [targetId, targetResolvedSID] = resolveNodeId(data.targetBaseId);

        df.m_type = data.type;

        df.m_mode = data.mode;
        df.m_speed = data.speed;

        df.m_targetId = targetId;

        df.m_direction = data.direction;
        df.m_distance = data.distance;

        return definition;
    }
}
