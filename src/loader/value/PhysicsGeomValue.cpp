#include "PhysicsGeomValue.h"

#include "util/util.h"
#include "util/glm_util.h"

#include "loader/document.h"
#include "loader/loader_util.h"

#include "PhysicsCategoryValue.h"

namespace loader {
    void PhysicsGeomValue::loadGeom(
        const loader::DocNode& node,
        loader::GeomData& data) const
    {
        data.enabled = true;

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
                else if (type == "height_field" || type == "height") {
                    data.type = physics::GeomType::height_field;
                }
                else if (type == "box" || type == "cube") {
                    data.type = physics::GeomType::box;
                }
                else if (type == "sphere" || type == "ball") {
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
                data.rotation = readDegreesRotation(v);
            }
            else if (k == "offset") {
                data.offset = readVec3(v);
            }
            //else if (k == "plane") {
            //    data.plane = readVec4(v);
            //}
            else if (k == "category" || k == "cat") {
                PhysicsCategoryValue loader;
                loader.loadMask(v, data.categoryMask);
            }
            else if (k == "collision" || k == "collide" || k == "coll" || k == "col") {
                PhysicsCategoryValue loader;
                loader.loadMask(v, data.collisionMask);
            }
            else if (k == "placeable" || k == "place") {
                data.placeable = readBool(v);
            }
            else {
                reportUnknown("geom_entry", k, v);
            }
        }
    }
}
