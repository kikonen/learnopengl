#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "ki/uuid.h"

#include "asset/Shader.h"

#include "registry/EntityType.h"

#include "BaseLoader.h"
#include "MaterialLoader.h"
#include "CustomMaterialLoader.h"
#include "SpriteLoader.h"
#include "CameraLoader.h"
#include "LightLoader.h"
#include "ControllerLoader.h"
#include "GeneratorLoader.h"
#include "PhysicsLoader.h"

class Registry;

class MeshType;
class Node;


namespace loader {
    struct MetaData {
        std::string name;

        std::string assetsDir;
        std::string modelsDir;
    };

    struct SkyboxData {
        std::string programName{ "skybox" };
        std::string materialName{};
        int priority{ -100 };

        bool gammaCorrect{ true };
        bool hdri{ false };
        bool swapFaces{ false };
        bool loadedFaces{ false };
        std::array<std::string, 6> faces;

        bool const valid() {
            return !materialName.empty();
        }
    };

    struct EntityCloneData {
        bool valid{ false };

        bool enabled{ false };

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
        glm::vec3 front{ 0.f, 0.f, 1.f };
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

        ControllerData controller;
        CameraData camera;
        LightData light;

        GeneratorData generator;
    };

    struct EntityData {
        bool isRoot{ false };
        EntityCloneData base;
        std::vector<EntityCloneData> clones;
    };

    class SceneLoader : public BaseLoader
    {
    public:
        SceneLoader(
            Context ctx);

        ~SceneLoader();

        void load(
            std::shared_ptr<Registry> registry);

    private:
        void attach(
            SkyboxData& skybox,
            const EntityData& root,
            const std::vector<EntityData>& entities);

        void attachSkybox(
            const EntityData& root,
            SkyboxData& data);

        void attachVolume(
            const EntityData& root);

        void attachCubeMapCenter(
            const EntityData& root);

        void attachEntity(
            const EntityData& root,
            const EntityData& data);

        MeshType* attachEntityClone(
            MeshType* type,
            const EntityData& root,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex);

        MeshType* attachEntityCloneRepeat(
            MeshType* type,
            const EntityData& root,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset);

        void assignFlags(
            const EntityCloneData& data,
            MeshType* type);

        MeshType* createType(
            bool isRoot,
            const EntityCloneData& data,
            const glm::uvec3& tile);

        void resolveMaterial(
            MeshType* type,
            const EntityCloneData& data);

        void resolveSprite(
            MeshType* type,
            const EntityCloneData& data);

        void resolveMesh(
            MeshType* type,
            const EntityCloneData& data,
            const glm::uvec3& tile);

        Node* createNode(
            MeshType* type,
            const EntityData& root,
            const EntityCloneData& data,
            const bool cloned,
            const int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& clonePositionOffset,
            const glm::vec3& tilePositionOffset,
            const bool isRoot);

        void loadMeta(
            const YAML::Node& node,
            MetaData& data);

        void loadSkybox(
            const YAML::Node& node,
            SkyboxData& data);

        void loadSkyboxFaces(
            const YAML::Node& node,
            SkyboxData& data);

        void loadRoot(
            const YAML::Node& doc,
            EntityData& root);

        void loadEntities(
            const YAML::Node& doc,
            std::vector<EntityData>& entities);

        void loadEntity(
            const YAML::Node& node,
            EntityData& data);

        void loadEntityClone(
            const YAML::Node& node,
            EntityCloneData& data,
            std::vector<EntityCloneData>& clones,
            bool recurse);

        void loadMaterialModifiers(
            const YAML::Node& node,
            EntityCloneData& data);

    public:

    private:
        MetaData m_meta;
        SkyboxData m_skybox;

        EntityData m_root;
        std::vector<EntityData> m_entities;

        MaterialLoader m_materialLoader;
        CustomMaterialLoader m_customMaterialLoader;
        SpriteLoader m_spriteLoader;

        CameraLoader m_cameraLoader;
        LightLoader m_lightLoader;
        ControllerLoader m_controllerLoader;
        GeneratorLoader m_generatorLoader;
        PhysicsLoader m_physicsLoader;
    };
}
