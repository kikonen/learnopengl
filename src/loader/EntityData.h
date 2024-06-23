#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include <glm/glm.hpp>

#include "asset/Shader.h"

#include "model/EntityType.h"

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

namespace loader {
    struct EntityData {
        bool enabled{ false };
        bool active{ false };

        EntityType type{ EntityType::none };

        BaseId baseId;
        BaseId parentBaseId;

        std::string name;
        std::string desc;

        std::string prefabName;

        int8_t priority{ 0 };

        std::vector<MeshData> meshes;

        std::string geometryType;
        std::unordered_map<MaterialProgramType, std::string> programs;

        loader::FlagContainer nodeFlags;
        loader::FlagContainer typeFlags;

        glm::vec3 baseRotation{ 0.f };

        glm::vec3 position{ 0.f };
        glm::vec3 rotation{ 0.f };
        // NOTE KI default == positive Z dir
        glm::vec3 front{ 0.f, 0.f, 1.f };
        glm::vec3 scale{ 1.f };
        glm::vec3 baseScale{ 1.f };

        bool selected{ false };
        bool cloneMesh{ true };

        Tiling tiling;
        glm::uvec3 tile{ 0 };

        glm::vec3 clonePositionOffset{ 0.f };

        CustomMaterialData customMaterial;
        PhysicsData physics;

        Repeat repeat;

        ScriptData script;

        std::vector<ControllerData> controllers;
        CameraData camera;
        LightData light;
        AudioData audio;
        TextData text;

        GeneratorData generator;

        ParticleData particle;
    };
}
