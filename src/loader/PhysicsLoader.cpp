#include "PhysicsLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"

#include "physics/Object.h"
#include "physics/PhysicsEngine.h"

namespace loader {
    PhysicsLoader::PhysicsLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void PhysicsLoader::loadPhysics(
        const YAML::Node& node,
        PhysicsData& data) const
    {
        bool explicitEnable = false;
        bool useExplicitEnable = false;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "enabled") {
                explicitEnable = readBool(v);
                useExplicitEnable = true;
            }
            else if (k == "body") {
                data.enabled = true;
                loadBody(v, data.body);
            }
            else if (k == "geom") {
                data.enabled = true;
                loadGeom(v, data.geom);
            }
            else {
                reportUnknown("physics_entry", k, v);
            }
        }

        // NOTE KI physics needs body or gem
        if (useExplicitEnable && data.enabled) {
            data.enabled = explicitEnable;
        }
    }

    void PhysicsLoader::loadBody(
        const YAML::Node& node,
        physics::Body& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = physics::BodyType::none;
                }
                else if (type == "sphere") {
                    data.type = physics::BodyType::sphere;
                }
                else if (type == "box") {
                    data.type = physics::BodyType::box;
                }
                else {
                    reportUnknown("body_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "density") {
                data.density = readFloat(v);
            }
            else if (k == "linear_vel") {
                data.linearVel = readVec3(v);
            }
            else if (k == "angular_vel") {
                data.angularVel = readVec3(v);
            }
            else if (k == "rotation") {
                data.rotation = readVec3(v);
            }
            else {
                reportUnknown("body_entry", k, v);
            }
        }
    }

    void PhysicsLoader::loadGeom(
        const YAML::Node& node,
        physics::Geom& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = physics::GeomType::none;
                }
                else if (type == "plane") {
                    data.type = physics::GeomType::plane;
                }
                else if (type == "sphere") {
                    data.type = physics::GeomType::sphere;
                }
                else if (type == "box") {
                    data.type = physics::GeomType::box;
                }
                else if (type == "capsule") {
                    data.type = physics::GeomType::capsule;
                }
                else if (type == "cylinder") {
                    data.type = physics::GeomType::cylinder;
                }
                else {
                    reportUnknown("geom_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "plane") {
                data.plane = readVec4(v);
            }
            else if (k == "category") {
                data.category = readInt(v);
            }
            else if (k == "collide") {
                data.collide = readInt(v);
            }
            else {
                reportUnknown("geom_entry", k, v);
            }
        }
    }

    std::unique_ptr<physics::Object> PhysicsLoader::createObject(
        const PhysicsData& data,
        Node* node)
    {
        if (!data.enabled) return nullptr;

        return std::make_unique<physics::Object>(data.body, data.geom);
    }
}
