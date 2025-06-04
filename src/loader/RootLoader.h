#pragma once

#include "BaseLoader.h"
#include "RootData.h"

namespace loader {
    class RootLoader : public BaseLoader
    {
    public:
        RootLoader(
            std::shared_ptr<Context> ctx);

        void loadRoot(
            const loader::DocNode& node,
            RootData& data) const;

        void attachRoot(
            const RootData& data);
    };
}
