#include "PrefabLoader.h"

#include "util/Util.h"
#include "util/glm_util.h"

#include "loader/document.h"

namespace loader {
    PrefabLoader::PrefabLoader(
        Context ctx)
        : BaseLoader(ctx)
    {}

    void PrefabLoader::loadPrefab(
        const loader::DocNode& node,
        PrefabData& data) const
    {
        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();
        }
    }
}
