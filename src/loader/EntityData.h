#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include <glm/glm.hpp>

#include "asset/Shader.h"

#include "registry/EntityType.h"

#include "BaseUUID.h"
#include "BaseData.h"
#include "MaterialData.h"
#include "CustomMaterialData.h"
#include "CameraData.h"
#include "LightData.h"
#include "ControllerData.h"
#include "GeneratorData.h"
#include "PhysicsData.h"

namespace loader {
    struct EntityCloneData {
        bool valid{ false };

        bool enabled{ false };
        bool active{ false };

        EntityType type{ EntityType::model };

        std::string name;
        std::string desc;

        int priority{ 0 };

        BaseUUID idBase;
        BaseUUID parentIdBase;

        std::string meshName;
        std::string meshPath;

        std::string programName{};
        std::string geometryType;

        std::string depthProgramName{ SHADER_DEPTH_PASS };

        std::map<std::string, std::string> programDefinitions{};
        std::unordered_map<std::string, bool> renderFlags{};
        glm::vec3 position{ 0.f };
        glm::vec3 rotation{ 0.f };
        glm::vec3 front{ 0.f, 0.f, -1.f };
        glm::vec3 scale{ 1.f };

        bool selected{ false };
        bool instanced{ false };
        bool cloneMesh{ true };

        Tiling tiling;
        glm::uvec3 tile{ 0 };

        glm::vec3 clonePositionOffset{ 0.f };

        bool loadTextures{ true };
        std::string materialName;
        // NOTE KI overrides *ALL* materials with defaultMaterial
        bool forceMaterial{ false };

        MaterialData materialModifiers;

        CustomMaterialData customMaterial;
        PhysicsData physics;

        std::string spriteName;

        int batchSize{ -1 };

        Repeat repeat;

        std::string script;

        std::vector<ControllerData> controllers;
        CameraData camera;
        LightData light;

        GeneratorData generator;
    };

    struct EntityData {
        bool isRoot{ false };
        EntityCloneData base;
        std::vector<EntityCloneData> clones;
    };
}
