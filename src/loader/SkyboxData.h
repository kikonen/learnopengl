#pragma once

#include <string>
#include <array>

namespace loader {
    struct SkyboxData {
        std::string programName{ "skybox" };
        std::string materialName;
        std::string materialName2;
        int priority{ -100 };

        bool gammaCorrect{ true };
        bool hdri{ false };
        bool swapFaces{ false };

        bool loadedFaces{ false };
        std::array<std::string, 6> faces;

        bool loadedFaces2{ false };
        std::array<std::string, 6> faces2;

        bool const valid() const {
            return !(materialName.empty() && materialName2.empty());
        }
    };
}
