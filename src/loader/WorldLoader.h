#pragma once

#include "BaseLoader.h"
#include "WorldData.h"

namespace loader
{
    struct ScriptSystemData;

    class WorldLoader : public BaseLoader
    {
    public:
        WorldLoader(
            const util::Ref<Context>& ctx);

        void loadWorld(
            const loader::DocNode& node,
            WorldData& data) const;

        void attachWorld(
            const WorldData& data,
            ScriptSystemData& scriptSystemData,
            Loaders& loaders);
    };
}
