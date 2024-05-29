#include "ControllerLoader.h"

#include "util/Util.h"

#include "model/Node.h"

#include "controller/PawnController.h"
#include "controller/CameraZoomController.h"
#include "controller/VolumeController.h"

#include "loader/document.h"

namespace loader {
    ControllerLoader::ControllerLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void ControllerLoader::loadControllers(
        const loader::Node& node,
        std::vector<ControllerData>& controllers) const
    {
        for (const auto& entry : node.getNodes()) {
            ControllerData& data = controllers.emplace_back();
            loadController(entry, data);
        }
    }

    void ControllerLoader::loadController(
        const loader::Node& node,
        ControllerData& data) const
    {
        data.enabled = true;

        // pos relative to owning node
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::Node& v = pair.getNode();

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
            else {
                reportUnknown("controller_entry", k, v);
            }
        }
    }

    NodeController* ControllerLoader::createController(
        const ControllerData& data,
        pool::NodeHandle handle)
    {
        if (!data.enabled) return nullptr;

        switch (data.type) {
        case ControllerType::pawn: {
            auto* controller = new PawnController();
            return controller;
        }
        case ControllerType::camera_zoom: {
            auto* controller = new CameraZoomController();
            return controller;
        }
        }

        return nullptr;
    }
}
