#pragma once

#include "BaseLoader.h"
#include "RootData.h"

namespace loader {
    struct ScriptSystemData;

    class RootLoader : public BaseLoader
    {
    public:
        RootLoader(
            const std::shared_ptr<Context>& ctx);

        void loadRoot(
            const loader::DocNode& node,
            RootData& data) const;

        void attachRoot(
            const RootData& data,
            ScriptSystemData& scriptSystemData,
            Loaders& loaders);
    };
}
