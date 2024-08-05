#include "PhysicsLoader.h"

#include <unordered_map>

#include "util/Util.h"
#include "util/glm_util.h"

#include "event/Dispatcher.h"

#include "loader/document.h"
#include "loader_util.h"

namespace {
    std::unordered_map<std::string, physics::Category> g_categoryMapping;

    const std::unordered_map<std::string, physics::Category>& getCategoryMapping()
    {
        if (g_categoryMapping.empty()) {
            g_categoryMapping.insert({
                { "none", physics::Category::none },
                { "invalid", physics::Category::invalid },
                // surroundings
                { "ground", physics::Category::ground },
                { "scenery", physics::Category::scenery },
                { "water", physics::Category::water },
                // characters and creatures
                { "npc", physics::Category::npc },
                { "player", physics::Category::player },
                // characteristic
                { "can_float", physics::Category::can_float },
                // ray
                { "ray_player_fire", physics::Category::ray_player_fire },
                { "ray_npc_fire", physics::Category::ray_npc_fire },
                { "ray_hit", physics::Category::ray_hit },
                });
        }
        return g_categoryMapping;
    }

    physics::Category readCategory(std::string v)
    {
        const auto& mapping = getCategoryMapping();
        const auto& it = mapping.find(v);
        if (it != mapping.end()) return it->second;
        // NOTE KI for data tracking data mismatches
        return physics::Category::invalid;
    }
}

namespace loader {
    PhysicsLoader::PhysicsLoader(
        Context ctx)
        : BaseLoader(ctx)
    {
    }

    void PhysicsLoader::loadPhysics(
        const loader::DocNode& node,
        PhysicsData& data) const
    {
        bool explicitEnable = false;
        bool useExplicitEnable = false;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "enabled") {
                explicitEnable = readBool(v);
                useExplicitEnable = true;
            }
            else if (k == "xxenabled" || k == "xenabled") {
                // NOTE compat with old "disable" logic
                explicitEnable = false;
                useExplicitEnable = true;
            }
            if (k == "update") {
                data.update = readBool(v);
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
        const loader::DocNode& node,
        physics::Body& data) const
    {
        data.quat = glm::quat_identity<float, glm::packed_highp>();

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = physics::BodyType::none;
                }
                else if (type == "box") {
                    data.type = physics::BodyType::box;
                }
                else if (type == "sphere") {
                    data.type = physics::BodyType::sphere;
                }
                else if (type == "capsule") {
                    data.type = physics::BodyType::capsule;
                }
                else if (type == "cylinder") {
                    data.type = physics::BodyType::cylinder;
                }
                else {
                    reportUnknown("body_type", k, v);
                }
            }
            else if (k == "kinematic") {
                data.kinematic = readBool(v);
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "scale") {
                data.scale = readScale3(v);
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
            else if (k == "rot" || k == "rotation") {
                data.quat = util::degreesToQuat(readDegreesRotation(v));
            }
            else {
                reportUnknown("body_entry", k, v);
            }
        }
    }

    void PhysicsLoader::loadGeom(
        const loader::DocNode& node,
        physics::Geom& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = physics::GeomType::none;
                }
                else if (type == "plane") {
                    data.type = physics::GeomType::plane;
                }
                else if (type == "box") {
                    data.type = physics::GeomType::box;
                }
                else if (type == "sphere") {
                    data.type = physics::GeomType::sphere;
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
            else if (k == "rot" || k == "rotation") {
                data.quat = util::degreesToQuat(readDegreesRotation(v));
            }
            else if (k == "plane") {
                data.plane = readVec4(v);
            }
            else if (k == "category") {
                loadMask(v, data.categoryMask);
            }
            else if (k == "collision") {
                loadMask(v, data.collisionMask);
            }
            else {
                reportUnknown("geom_entry", k, v);
            }
        }
    }

    void PhysicsLoader::loadMask(
        const loader::DocNode& node,
        uint32_t& mask) const
    {
        uint32_t m = 0;
        for (const auto& entry : node.getNodes()) {
            const auto& category = readCategory(readString(entry));
            m |= util::as_integer(category);
        }
        mask = m;
    }

    void PhysicsLoader::createObject(
        const PhysicsData& data,
        const pool::NodeHandle handle)
    {
        if (!data.enabled) return;

        {
            event::Event evt { event::Type::physics_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body.physics = {
                .update = data.update,
                .body = data.body,
                .geom = data.geom,
            };
            auto& body = evt.body.physics = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }
    }
}
