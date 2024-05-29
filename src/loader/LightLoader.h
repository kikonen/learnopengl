#pragma once

#include "BaseLoader.h"
#include "LightData.h"

namespace loader {
    class LightLoader : public BaseLoader
    {
    public:
        LightLoader(
            Context ctx);

        void loadLight(
            const loader::Node& node,
            LightData& data) const;

        std::unique_ptr<Light> createLight(
            const LightData& data,
            const int cloneIndex,
            const glm::uvec3& tile);
    };
}
