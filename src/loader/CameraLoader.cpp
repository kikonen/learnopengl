#include "CameraLoader.h"

#include "asset/Assets.h"

#include "util/Util.h"

#include "component/FpsCamera.h"
#include "component/FollowCamera.h"
#include "component/OrbitCamera.h"

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

        data.enabled = false;
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

    std::unique_ptr<CameraComponent> CameraLoader::createCamera(
        const CameraData& data)
    {
        if (!data.enabled) return nullptr;

        // NOTE only node cameras in scenefile for now
        std::unique_ptr<CameraComponent> component;

        switch (data.type) {
        case CameraType::fps:
            component = std::make_unique<FpsCamera>();
            break;
        case CameraType::follow: {
            auto followCamera = std::make_unique<FollowCamera>();
            followCamera->m_springConstant = data.springConstant;
            followCamera->m_distance = data.distance;
            component = std::move(followCamera);
            break;
        }
        case CameraType::orbit: {
            auto orbitCamera = std::make_unique<OrbitCamera>();
            orbitCamera->m_offset = data.offset;
            orbitCamera->m_up = data.up;
            orbitCamera->m_pitchSpeed = glm::radians(data.pitchSpeed);
            orbitCamera->m_yawSpeed = glm::radians(data.yawSpeed);
            component = std::move(orbitCamera);
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
