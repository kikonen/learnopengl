#include "CameraLoader.h"

#include "asset/Assets.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "component/Camera.h"

namespace loader
{
    CameraLoader::CameraLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void CameraLoader::loadCamera(
        const YAML::Node& node,
        CameraData& data) const
    {
        const auto& assets = Assets::get();

        data.fov = assets.cameraFov;

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
            else if (k == "default") {
                data.isDefault = readBool(v);
            }
            else if (k == "fov") {
                data.fov = readFloat(v);
            }
            else if (k == "front") {
                data.front = readVec3(v);
            }
            else if (k == "up") {
                data.up = readVec3(v);
            }
            else if (k == "pos") {
                throw std::runtime_error{ fmt::format("POS obsolete: {}", renderNode(node)) };
            }
            else if (k == "rot" || k == "rotation") {
                data.degreesRotation = readVec3(v);
            }
            else if (k == "orthagonal") {
                data.orthagonal = readBool(v);
            }
            else if (k == "viewport") {
                const auto& vec = readVec4(v);
                data.viewport = { vec[0], vec[1], vec[2], vec[3] };
            }
            else {
                reportUnknown("controller_entry", k, v);
            }
        }
    }

    std::unique_ptr<Camera> CameraLoader::createCamera(
        const CameraData& data)
    {
        if (!data.enabled) return std::unique_ptr<Camera>();

        // NOTE only node cameras in scenefile for now
        auto camera = std::make_unique<Camera>();

        if (data.orthagonal) {
            camera->setViewport(data.viewport);
        }
        camera->setAxis(data.front, data.up);
        camera->setDegreesRotation(data.degreesRotation);
        camera->setFov(data.fov);

        camera->setEnabled(data.enabled);
        camera->setDefault(data.isDefault);

        return camera;
    }
}
