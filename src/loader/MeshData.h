#pragma once

#include <string>
#include <vector>

#include "util/Axis.h"

#include "FlagContainer.h"

#include "LodData.h"
#include "RigData.h"
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

        std::string bindRig;

        // NOTE KI default to smooth normals (usually better)
        bool smoothNormals{ true };
        // NOTE KI don't force
        bool forceNormals{ false };

        glm::vec3 scale{ 1.f };
        glm::vec3 baseScale{ 1.f };

        util::Axis baseAxis{ util::Axis::y };
        util::Front baseFront{ util::Front::z };
        glm::vec3 baseAdjust{ 0.f };  // additional rotation in degrees

        std::vector<LodData> lods;

        std::vector<RigData> rigs;

        bool defaultPrograms{ false };
        std::unordered_map<MaterialProgramType, std::string> programs;

        std::vector<MaterialData> materials;
        std::vector<MaterialData> materialModifiers;

        loader::FlagContainer meshFlags;

        VertexData vertexData;

        const LodData* findLod(
            const std::string& meshName) const noexcept
        {
            const LodData* any = nullptr;
            for (const auto& lod : lods) {
                if (lod.isAny()) {
                    any = &lod;
                    continue;
                }
                if (lod.match(meshName)) return &lod;
            }

            return any;
        }
    };
}
