#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>

#include <glm/glm.hpp>

#include "asset/Shader.h"

#include "mesh/EntityType.h"

#include "BaseId.h"
#include "BaseData.h"

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
#include "LodData.h"
#include "AnimationData.h"

namespace loader {
    struct EntityData {
        bool enabled{ false };
        bool active{ false };

        mesh::EntityType type{ mesh::EntityType::model };

        BaseId baseId;
        BaseId parentBaseId;

        std::string name;
        std::string desc;

        std::string prefabName;

        int priority{ 0 };

        std::vector<LodData> lods;

        std::string programName{};
        std::string geometryType;

        std::string shadowProgramName;
        std::string preDepthProgramName{ SHADER_PRE_DEPTH_PASS };

        std::map<std::string, std::string> programDefinitions{};
        std::unordered_map<std::string, bool> renderFlags{};

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

        std::string spriteName;

        Repeat repeat;

        ScriptData script;

        std::vector<ControllerData> controllers;
        CameraData camera;
        LightData light;
        AudioData audio;
        TextData text;

        std::vector<AnimationData> animations;

        GeneratorData generator;

        ParticleData particle;

        bool findRenderFlag(
            const std::string& flag,
            bool defaultValue) const noexcept
        {
            const auto& e = renderFlags.find(flag);
            if (e != renderFlags.end()) {
                return e->second;
            }
            return defaultValue;
        }
    };
}
