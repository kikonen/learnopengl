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
            const std::shared_ptr<Context>& ctx);

        void loadPrefab(
            const loader::DocNode& node,
            PrefabData& data) const;
    };
}
