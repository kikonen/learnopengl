#include "PhysicsLoader.h"

#include <unordered_map>

#include <ode/ode.h>

#include "util/debug.h"
#include "util/Log.h"
#include "util/util.h"
#include "util/glm_util.h"

#include "component/definition/PhysicsDefinition.h"

#include "physics/physics_util.h"

#include "event/Dispatcher.h"

#include "loader/document.h"
#include "loader_util.h"

#include "value/PhysicsCategoryValue.h"
#include "value/PhysicsGeomValue.h"

namespace {
}

namespace loader {
    PhysicsLoader::PhysicsLoader(
        std::shared_ptr<Context> ctx)
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
                PhysicsGeomValue loader;
                loader.loadGeom(v, data.geom);
            }
            else {
                reportUnknown("physics_entry", k, v);
            }
        }

        if (data.body.type == physics::BodyType::none && data.geom.type == physics::GeomType::none)
        {
            data.enabled = false;
        }


        // NOTE KI physics needs body or gem
        if (useExplicitEnable) {
            data.enabled = explicitEnable;
        }
    }

    void PhysicsLoader::loadBody(
        const loader::DocNode& node,
        loader::BodyData& data) const
    {
        //data.baseRotation = glm::quat_identity<float, glm::packed_highp>();

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
            else if (k == "density") {
                data.density = readFloat(v);
            }
            else if (k == "linear_velocity" || k == "linear_vel") {
                data.linearVelocity = readVec3(v);
            }
            else if (k == "angular_velocity" || k == "angular_vel") {
                data.angularVelocity = readVec3(v);
            }
            else if (k == "max_angular_velocity" || k == "max_angular_vel") {
                data.maxAngulerVelocity = readFloat(v);
            }
            else if (k == "rot" || k == "rotation") {
                data.baseRotation = readDegreesRotation(v);
            }
            else if (k == "axis") {
                data.axis = readVec3(v);
            }
            else if (k == "force_axis") {
                data.forceAxis = readBool(v);
            }
            else {
                reportUnknown("body_entry", k, v);
            }
        }
    }

    std::unique_ptr<PhysicsDefinition> PhysicsLoader::createPhysicsDefinition(
        const loader::PhysicsData& data)
    {
        if (!data.enabled) return nullptr;

        auto definition = std::make_unique<PhysicsDefinition>();
        auto& df = *definition;

        df.m_update = data.update;

        {
            auto& bodyData = data.body;
            auto& body = df.m_body;

            body.m_size = bodyData.size;

            body.m_baseRotation = util::degreesToQuat(bodyData.baseRotation);

            body.m_linearVelocity = bodyData.linearVelocity;
            body.m_angularVelocity = bodyData.angularVelocity;

            body.m_axis = bodyData.axis;

            body.m_maxAngulerVelocity = bodyData.maxAngulerVelocity;
            body.m_density = bodyData.density;

            body.m_type = bodyData.type;

            body.m_forceAxis = bodyData.forceAxis;
            body.m_kinematic = bodyData.kinematic;
        }

        {
            auto& geomData = data.geom;
            auto& geom = df.m_geom;

            geom.m_size = geomData.size;

            geom.m_rotation = util::degreesToQuat(geomData.rotation);
            geom.m_offset = geomData.offset;

            geom.m_categoryMask = geomData.categoryMask;
            geom.m_collisionMask = geomData.collisionMask;

            geom.m_type = geomData.type;

            geom.m_placeable = geomData.placeable;
        }

        return definition;
    }
}
