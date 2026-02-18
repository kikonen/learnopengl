#pragma once

#include <string>
#include <vector>

#include "asset/LayerInfo.h"

#include "material/Material.h"

#include "model/PivotPoint.h"

#include "FlagContainer.h"
#include "NodeKind.h"
#include "MeshData.h"
#include "CameraData.h"
#include "LightData.h"
#include "AudioData.h"
#include "ControllerData.h"
#include "GeneratorData.h"
#include "ScriptData.h"
#include "TextData.h"
#include "MaterialData.h"
#include "CustomMaterialData.h"
#include "AddonData.h"

namespace loader
{
    struct NodeTypeData {
        bool enabled{ false };

        NodeKind type{ NodeKind::none };

        BaseId baseId;
        BaseId parentId;
        BaseId compositeId;

        std::string name;
        std::string desc;

        std::string prefabName;

        uint8_t layer{ LAYER_NONE_INDEX };
        int8_t priority{ 0 };

        std::vector<MeshData> meshes;
        std::vector<MeshData> availableAddons;
        std::vector<AddonData> addons;

        std::unordered_map<MaterialProgramType, std::string> programs;

        loader::FlagContainer typeFlags;

        glm::vec3 baseRotation{ 0.f };
        glm::vec3 baseScale{ 1.f };

        // NOTE KI default == positive Z dir
        glm::vec3 front{ 0.f, 0.f, 1.f };

        model::PivotPoint pivot;

        CustomMaterialData customMaterial;
        PhysicsData physics;

        std::vector<ScriptData> scripts;

        std::vector<ControllerData> controllers;
        CameraData camera;
        LightData light;
        AudioData audio;
        TextData text;
        BaseId particleId;

        GeneratorData generator;

        const std::string& str() const noexcept
        {
            return baseId.str();
        }
    };
}
