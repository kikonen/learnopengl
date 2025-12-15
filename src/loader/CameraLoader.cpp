#include "CameraLoader.h"

#include "asset/Assets.h"

#include "util/glm_util.h"
#include "util/util.h"

#include "component/definition/CameraComponentDefinition.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader
{
    CameraLoader::CameraLoader(
        const std::shared_ptr<Context>& ctx)
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
            else if (k == "pitch") {
                data.pitch = readFloat(v);
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
            else if (k == "orthogonal") {
                data.orthogonal = readBool(v);
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

    std::unique_ptr<CameraComponentDefinition> CameraLoader::createDefinition(
        const CameraData& data)
    {
        if (!data.enabled) return nullptr;

        auto definition = std::make_unique<CameraComponentDefinition>();
        auto& df = *definition;

        df.m_type = data.type;

        df.m_default = data.isDefault;

        df.m_orthogonal = data.orthogonal;
        df.m_viewport = data.viewport;

        df.m_fov = data.fov;

        df.m_front = data.front;
        df.m_up = data.up;

        df.m_offset = data.offset;

        df.m_pitch = data.pitch;
        df.m_pitchSpeed = data.pitchSpeed;
        df.m_yawSpeed = data.yawSpeed;

        df.m_distance = data.distance;

        df.m_springConstant = data.springConstant;

        df.m_path = data.path;
        df.m_speed = data.speed;

        return definition;
    }
}
