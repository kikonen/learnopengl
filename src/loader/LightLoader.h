#pragma once

#include "BaseLoader.h"

class Light;

namespace loader {
    enum class LightType {
        none,
        directional,
        point,
        spot
    };

    struct LightData {
        bool enabled{ false };
        LightType type{ LightType::none };

        glm::vec3 pos{ 0.f };

        BaseUUID targetIdBase;

        float linear{ 0.f };
        float quadratic{ 0.f };

        float cutoffAngle{ 0.f };
        float outerCutoffAngle{ 0.f };

        glm::vec3 diffuse{ 0.5f, 0.5f, 0.5f };
        float intensity{ 1.f };
    };

    class LightLoader : public BaseLoader
    {
    public:
        LightLoader(
            Context ctx);

        void loadLight(
            const YAML::Node& node,
            LightData& data);

        std::unique_ptr<Light> createLight(
            const LightData& data,
            const int cloneIndex,
            const glm::uvec3& tile);
    };
}
