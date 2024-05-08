#include "PrefabLoader.h"

#include "ki/yaml.h"
#include "util/Util.h"
#include "util/glm_util.h"

namespace loader {
    PrefabLoader::PrefabLoader(
        Context ctx)
        : BaseLoader(ctx)
    {}

    void PrefabLoader::loadPrefab(
        const YAML::Node& node,
        PrefabData& data) const
    {
    }
}
