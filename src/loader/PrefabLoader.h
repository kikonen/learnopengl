#pragma once

#include <vector>

#include "ki/size.h"

#include "BaseLoader.h"

#include "PrefabData.h"

namespace loader {
    class PrefabLoader : public BaseLoader
    {
    public:
        PrefabLoader(
            Context ctx);

        void loadPrefab(
            const YAML::Node& node,
            PrefabData& data) const;
    };
}
