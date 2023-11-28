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
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            // NOTE KI physics needs body or gem
            if (k == "space") {
                data.space = readString(v);
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
    }

    void PhysicsLoader::loadBody(
        const YAML::Node& node,
        BodyData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = BodyType::none;
                }
                else if (type == "sphere") {
                    data.type = BodyType::sphere;
                }
                else if (type == "box") {
                    data.type = BodyType::box;
                }
                else {
                    reportUnknown("body_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "mass") {
                data.mass = readFloat(v);
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
        GeomData& data) const
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = GeomType::none;
                }
                else if (type == "plane") {
                    data.type = GeomType::plane;
                }
                else if (type == "sphere") {
                    data.type = GeomType::sphere;
                }
                else if (type == "box") {
                    data.type = GeomType::box;
                }
                else if (type == "capsule") {
                    data.type = GeomType::capsule;
                }
                else if (type == "cylinder") {
                    data.type = GeomType::cylinder;
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

    std::unique_ptr<physics::Object> PhysicsLoader::createPhysicsObject(
        const PhysicsData& data,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        //switch (data.type) {
        //case CustomMaterialType::text: {
        //    auto material{ std::make_unique<TextMaterial>() };
        //    material->m_fontName = data.fontName;
        //    material->m_fontSize = data.fontSize;

        //    return material;
        //}
        //}

        return nullptr;
    }

}
