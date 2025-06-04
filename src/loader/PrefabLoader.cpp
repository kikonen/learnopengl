#include "PrefabLoader.h"

#include "util/util.h"
#include "util/glm_util.h"

#include "loader/document.h"
#include "loader_util.h"

namespace loader {
    PrefabLoader::PrefabLoader(
        std::shared_ptr<Context> ctx)
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
