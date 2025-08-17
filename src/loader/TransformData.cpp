#include "TransformData.h"

#include "util/Util.h"

#include "loader_util.h"

namespace loader {
    void TransformData::load(const loader::DocNode& node)
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& key = pair.getName();
            const loader::DocNode& v = pair.getNode();

            const auto k = util::toLower(key);

            if (k == "position" || k == "offset") {
                position = readVec3(v);
            }
            else if (k == "rotation") {
                rotation = readDegreesRotation(v);
            }
            else if (k == "scale") {
                scale = readScale3(v);
            }
            else {
                reportUnknown("transform_entry", k, v);
            }
        }
    }
}
