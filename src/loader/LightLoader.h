#pragma once

#include "BaseLoader.h"
#include "LightData.h"

namespace loader {
    class LightLoader : public BaseLoader
    {
    public:
        LightLoader(
            const std::shared_ptr<Context>& ctx);

        void loadLight(
            const loader::DocNode& node,
            LightData& data) const;

        std::unique_ptr<LightDefinition> createDefinition(
            const LightData& data) const;
    };
}
