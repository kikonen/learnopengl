#pragma once

#include "BaseLoader.h"
#include "RootData.h"

namespace loader {
    class RootLoader : public BaseLoader
    {
    public:
        RootLoader(
            Context ctx);

        void loadRoot(
            const loader::Node& node,
            RootData& data) const;

        void attachRoot(
            const RootData& data);
    };
}
