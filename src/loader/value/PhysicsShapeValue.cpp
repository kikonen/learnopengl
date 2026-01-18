#include "PhysicsShapeValue.h"

#include "util/util.h"
#include "util/glm_util.h"

#include "loader/document.h"
#include "loader/loader_util.h"

#include "PhysicsCategoryValue.h"
#include "AxisValue.h"
#include "FrontValue.h"

namespace loader {
    void PhysicsShapeValue::loadShape(
        const loader::DocNode& node,
        loader::ShapeData& data) const
    {
        data.enabled = true;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = physics::ShapeType::none;
                }
                else if (type == "plane") {
                    data.type = physics::ShapeType::plane;
                }
                else if (type == "height_field" || type == "height") {
                    data.type = physics::ShapeType::height_field;
                }
                else if (type == "box" || type == "cube") {
                    data.type = physics::ShapeType::box;
                }
                else if (type == "sphere" || type == "ball") {
                    data.type = physics::ShapeType::sphere;
                }
                else if (type == "capsule") {
                    data.type = physics::ShapeType::capsule;
                }
                else if (type == "cylinder") {
                    data.type = physics::ShapeType::cylinder;
                }
                else {
                    reportUnknown("shape_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "axis") {
                data.baseAxis = AxisValue::load(v);
            }
            else if (k == "front") {
                data.baseFront = FrontValue::load(v);
            }
            else if (k == "adjust") {
                data.baseAdjust = readVec3(v);
            }
            else if (k == "offset") {
                data.offset = readVec3(v);
            }
            else if (k == "category" || k == "cat") {
                PhysicsCategoryValue loader;
                data.category = loader.loadCategory(v);
            }
            else if (k == "collision" || k == "collide" || k == "coll" || k == "col") {
                PhysicsCategoryValue loader;
                loader.loadMask(v, data.collisionMask);
            }
            else if (k == "placeable" || k == "place") {
                data.placeable = readBool(v);
            }
            else {
                reportUnknown("shape_entry", k, v);
            }
        }
    }
}
