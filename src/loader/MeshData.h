#pragma once

#include <string>
#include <vector>

#include "FlagContainer.h"

#include "LodData.h"
#include "SocketData.h"
#include "AnimationData.h"
#include "MaterialData.h"
#include "VertexData.h"

#include "MeshDataType.h"

namespace loader {
    struct MeshData {
        bool explicitEnabled{ false };
        bool enabled{ false };

        MeshDataType type{ MeshDataType::mesh };

        std::string baseDir;

        std::string id;
        std::string name;
        std::string path;

        // NOTE KI default to smooth normals (usually better)
        bool smoothNormals{ true };
        // NOTE KI don't force
        bool forceNormals{ false };

        glm::vec3 scale{ 1.f };
        glm::vec3 baseScale{ 1.f };

        glm::vec3 baseRotation{ 0.f };

        std::vector<LodData> lods;
        std::vector<SocketData> sockets;

        bool defaultPrograms{ false };
        std::unordered_map<MaterialProgramType, std::string> programs;

        std::vector<MaterialData> materials;
        std::vector<MaterialData> materialModifiers;
        std::vector<AnimationData> animations;

        loader::FlagContainer meshFlags;

        VertexData vertexData;

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
