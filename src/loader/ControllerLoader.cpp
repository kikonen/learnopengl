#include "ControllerLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "model/Node.h"

#include "controller/PawnController.h"
#include "controller/CameraController.h"
#include "controller/VolumeController.h"


namespace loader {
    ControllerLoader::ControllerLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void ControllerLoader::loadController(
        const YAML::Node& node,
        ControllerData& data) const
    {
        // pos relative to owning node
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = ControllerType::none;
                }
                else if (type == "pawn") {
                    data.type = ControllerType::pawn;
                }
                else if (type == "camera") {
                    data.type = ControllerType::camera;
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
            else {
                reportUnknown("controller_entry", k, v);
            }
        }
    }

    NodeController* ControllerLoader::createController(
        const ControllerData& data,
        Node* node)
    {
        if (!data.enabled) return nullptr;

        const auto& center = node->getPosition();

        switch (data.type) {
        case ControllerType::pawn: {
            auto* controller = new PawnController();
            return controller;
        }
        case ControllerType::camera: {
            auto* controller = new CameraController();
            return controller;
        }
        }

        return nullptr;
    }
}
