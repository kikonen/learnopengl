#include "CameraLoader.h"

#include "asset/Assets.h"

#include "util/util.h"

#include "component/FpsCamera.h"
#include "component/FollowCamera.h"
#include "component/OrbitCamera.h"
#include "component/SplineCamera.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader
{
    CameraLoader::CameraLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void CameraLoader::loadCamera(
        const loader::DocNode& node,
        CameraData& data) const
    {
        const auto& assets = Assets::get();

        data.enabled = true;
        data.fov = assets.cameraFov;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type" || k == "xtype") {
                data.enabled = k != "xtype";

                std::string type = readString(v);
                if (type == "none") {
                    data.type = CameraType::none;
                }
                else if (type == "fps") {
                    data.type = CameraType::fps;
                }
                else if (type == "follow") {
                    data.type = CameraType::follow;
                }
                else if (type == "orbit") {
                    data.type = CameraType::orbit;
                }
                else if (type == "spline") {
                    data.type = CameraType::spline;
                }
                else {
                    data.enabled = false;
                    reportUnknown("camera_type", k, v);
                }
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
            else if (k == "offset") {
                data.offset = readVec3(v);
            }
            else if (k == "pitch_speed") {
                data.pitchSpeed = readFloat(v);
            }
            else if (k == "yaw_speed") {
                data.yawSpeed = readFloat(v);
            }
            else if (k == "distance") {
                data.distance = readVec3(v);
            }
            else if (k == "spring_constant") {
                data.springConstant = readFloat(v);
            }
            else if (k == "path") {
                loadPath(v, data.path);
            }
            else if (k == "speed") {
                data.speed = readFloat(v);
            }
            else if (k == "pos") {
                throw std::runtime_error{ fmt::format("POS obsolete: {}", renderNode(node)) };
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

    void CameraLoader::loadPath(
        const loader::DocNode& node,
        std::vector<glm::vec3>& path) const
    {
        for (const auto& entry : node.getNodes()) {
            auto& point = path.emplace_back();
            point = readVec3(entry);
        }
    }

    std::unique_ptr<CameraComponent> CameraLoader::createCamera(
        const CameraData& data)
    {
        if (!data.enabled) return nullptr;

        // NOTE only node cameras in scenefile for now
        std::unique_ptr<CameraComponent> component;

        switch (data.type) {
        case CameraType::fps: {
            auto c = std::make_unique<FpsCamera>();
            component = std::move(c);
            break;
        }
        case CameraType::follow: {
            auto c = std::make_unique<FollowCamera>();
            c->m_springConstant = data.springConstant;
            c->m_distance = data.distance;
            component = std::move(c);
            break;
        }
        case CameraType::orbit: {
            auto c = std::make_unique<OrbitCamera>();
            c->m_offset = data.offset;
            c->m_up = data.up;
            c->m_pitchSpeed = glm::radians(data.pitchSpeed);
            c->m_yawSpeed = glm::radians(data.yawSpeed);
            component = std::move(c);
            break;
        }
        case CameraType::spline: {
            auto c = std::make_unique<SplineCamera>();
            c->m_path = Spline{ data.path };
            c->m_speed = data.speed;
            component = std::move(c);
            break;
        }
        }

        component->m_enabled = data.enabled;
        component->m_default = data.isDefault;

        {
            auto& camera = component->getCamera();
            if (data.orthagonal) {
                camera.setViewport(data.viewport);
            }
            camera.setAxis(data.front, data.up);
            camera.setFov(data.fov);
        }

        return component;
    }
}
