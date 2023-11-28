#pragma once

#include "BaseLoader.h"

namespace loader {
    struct SkyboxData {
        std::string programName{ "skybox" };
        std::string materialName{};
        int priority{ -100 };

        bool gammaCorrect{ true };
        bool hdri{ false };
        bool swapFaces{ false };
        bool loadedFaces{ false };
        std::array<std::string, 6> faces;

        bool const valid() const {
            return !materialName.empty();
        }
    };

    class SkyboxLoader : public BaseLoader
    {
    public:
        SkyboxLoader(
            Context ctx);

        void loadSkybox(
            const YAML::Node& node,
            SkyboxData& data);

        void loadSkyboxFaces(
            const YAML::Node& node,
            SkyboxData& data);

        void attachSkybox(
            const uuids::uuid& rootId,
            const SkyboxData& data);
    };
}
