#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>

#include "ki/uuid.h"

#include "asset/Shader.h"

#include "registry/EntityType.h"

#include "BaseLoader.h"
#include "RootLoader.h"
#include "SkyboxLoader.h"
#include "VolumeLoader.h"
#include "CubeMapLoader.h"
#include "MaterialLoader.h"
#include "CustomMaterialLoader.h"
#include "SpriteLoader.h"
#include "CameraLoader.h"
#include "LightLoader.h"
#include "ControllerLoader.h"
#include "GeneratorLoader.h"
#include "PhysicsLoader.h"
#include "EntityLoader.h"

class Registry;

class MeshType;
class Node;


namespace loader {
    struct MetaData {
        std::string name;

        //std::string assetsDir;
        //std::string modelsDir;
    };

    class SceneLoader : public BaseLoader
    {
    public:
        SceneLoader(
            Context ctx);

        ~SceneLoader();

        void prepare(
            std::shared_ptr<Registry> registry);

        void load();

    private:
        void attach(
            const RootData& root);

        void attachEntity(
            const uuids::uuid& rootId,
            const EntityData& data);

        MeshType* attachEntityClone(
            MeshType* type,
            const uuids::uuid& rootId,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex);

        MeshType* attachEntityCloneRepeat(
            MeshType* type,
            const uuids::uuid& rootId,
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
            const uuids::uuid& rootId,
            const EntityCloneData& data,
            const bool cloned,
            const int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& clonePositionOffset,
            const glm::vec3& tilePositionOffset,
            const bool isRoot);

        void loadMeta(
            const YAML::Node& node,
            MetaData& data) const;

        Material* findMaterial(
            std::string_view name);

        Sprite* findSprite(
            std::string_view name);

    private:
        MetaData m_meta;
        SkyboxData m_skybox;

        RootData m_root;
        std::vector<EntityData> m_entities;

        Material m_defaultMaterial;
        std::vector<MaterialData> m_materials;

        std::vector<SpriteData> m_sprites;

        RootLoader m_rootLoader;

        SkyboxLoader m_skyboxLoader;
        VolumeLoader m_volumeLoader;
        CubeMapLoader m_cubeMapLoader;

        EntityLoader m_entityLoader;

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
