#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include <glm/glm.hpp>

#include "asset/LayerInfo.h"

#include "shader/Shader.h"

#include "model/NodeType.h"
#include "model/PivotPoint.h"

#include "BaseId.h"
#include "BaseData.h"
#include "FlagContainer.h"

#include "MaterialData.h"
#include "CustomMaterialData.h"
#include "CameraData.h"
#include "LightData.h"
#include "AudioData.h"
#include "ControllerData.h"
#include "GeneratorData.h"
#include "ParticleData.h"
#include "PhysicsData.h"
#include "ScriptData.h"
#include "TextData.h"
#include "MeshData.h"
#include "AttachmentData.h"


namespace loader {
    struct NodeData {
        bool enabled{ false };
        bool active{ false };

        NodeType type{ NodeType::none };

        BaseId baseId;
        BaseId parentBaseId;
        BaseId ignoredByBaseId;

        std::string name;
        std::string desc;

        std::string prefabName;

        uint8_t layer{ LAYER_NONE_INDEX };
        int8_t priority{ 0 };

        std::vector<MeshData> meshes;
        std::vector<AttachmentData> attachments;

        std::unordered_map<MaterialProgramType, std::string> programs;

        loader::FlagContainer nodeFlags;
        loader::FlagContainer typeFlags;

        glm::vec3 baseRotation{ 0.f };

        glm::vec3 position{ 0.f };
        glm::vec3 rotation{ 0.f };
        // NOTE KI default == positive Z dir
        glm::vec3 front{ 0.f, 0.f, 1.f };
        glm::vec3 scale{ 1.f };
        glm::vec3 meshScale{ 1.f };

        PivotPoint pivot;

        bool selected{ false };
        bool cloneMesh{ true };

        Tiling tiling;
        glm::uvec3 tile{ 0 };

        glm::vec3 clonePositionOffset{ 0.f };

        CustomMaterialData customMaterial;
        PhysicsData physics;

        Repeat repeat;

        std::vector<ScriptData> scripts;

        std::vector<ControllerData> controllers;
        CameraData camera;
        LightData light;
        AudioData audio;
        TextData text;

        GeneratorData generator;

        ParticleData particle;

        const std::string& str() const noexcept
        {
            return baseId.str();
        }
    };
}
