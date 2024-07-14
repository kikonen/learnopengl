#pragma once

#include <string>
#include <vector>

#include "FlagContainer.h"

#include "LodData.h"
#include "SocketData.h"
#include "AnimationData.h"
#include "MaterialData.h"

namespace loader {
    struct MeshData {
        bool enabled{ false };

        std::string baseDir;

        std::string name;
        std::string path;

        // NOTE KI used *ONLY* if explicitly set to valid value
        // => otherwise value from NodeData is inherited
        glm::vec3 scale{ 0.f };

        glm::vec3 baseScale{ 1.f };

        std::vector<LodData> lods;
        std::vector<SocketData> sockets;

        std::vector<MaterialData> materials;
        std::vector<AnimationData> animations;

        loader::FlagContainer meshFlags;

        const LodData* findLod(
            const std::string& meshName) const noexcept
        {
            const LodData* any = nullptr;
            for (const auto& lod : lods) {
                if (lod.name == "*") {
                    any = &lod;
                    continue;
                }
                if (lod.name == meshName) return &lod;
            }

            return any;
        }
    };
}
