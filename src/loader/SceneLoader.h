#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include "asset/Shader.h"

#include "mesh/EntityType.h"

#include "BaseLoader.h"
#include "RootLoader.h"
#include "ScriptLoader.h"
#include "SkyboxLoader.h"
#include "VolumeLoader.h"
#include "CubeMapLoader.h"
#include "FontLoader.h"
#include "MaterialLoader.h"
#include "CustomMaterialLoader.h"
#include "SpriteLoader.h"
#include "CameraLoader.h"
#include "LightLoader.h"
#include "ControllerLoader.h"
#include "AudioLoader.h"
#include "GeneratorLoader.h"
#include "PhysicsLoader.h"
#include "EntityLoader.h"

class Registry;

namespace mesh {
    class MeshType;
}

namespace pool {
    class NodeHandle;
}

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

        void destroy();

        bool isRunning();

        void prepare(
            std::shared_ptr<Registry> registry);

        void load();

    private:
        void loadedEntity(
            const EntityData& data,
            bool success);

        void attach(
            const RootData& root);

        bool attachEntity(
            const ki::node_id rootId,
            const EntityData& data);

        const mesh::MeshType* attachEntityClone(
            const mesh::MeshType* type,
            const ki::node_id rootId,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex);

        const mesh::MeshType* attachEntityCloneRepeat(
            const mesh::MeshType* type,
            const ki::node_id rootId,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset);

        void assignFlags(
            const EntityCloneData& data,
            mesh::MeshType* type);

        const mesh::MeshType* createType(
            const EntityCloneData& data,
            const glm::uvec3& tile);

        void resolveProgram(
            mesh::MeshType* type,
            const EntityCloneData& data);

        text::font_id resolveFont(
            const mesh::MeshType* type,
            const TextData& data) const;

        void resolveMaterial(
            mesh::MeshType* type,
            const EntityCloneData& data);

        void modifyMaterials(
            mesh::MeshType* type,
            const EntityCloneData& data);

        void resolveSprite(
            mesh::MeshType* type,
            const EntityCloneData& data);

        void resolveMesh(
            mesh::MeshType* type,
            const EntityCloneData& data,
            const glm::uvec3& tile);

        pool::NodeHandle createNode(
            const mesh::MeshType* type,
            const ki::node_id rootId,
            const EntityCloneData& data,
            const bool cloned,
            const int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& clonePositionOffset,
            const glm::vec3& tilePositionOffset);

        void loadMeta(
            const YAML::Node& node,
            MetaData& data) const;

        void validate(
            const RootData& root);

        void validateEntity(
            const ki::node_id rootId,
            const EntityData& data,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateEntityClone(
            const ki::node_id rootId,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateEntityCloneRepeat(
            const ki::node_id rootId,
            const EntityData& entity,
            const EntityCloneData& data,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        const Material * findMaterial(
            std::string_view name) const;

        const Sprite* findSprite(
            std::string_view name) const;

        const FontData* findFont(
            std::string_view name) const;

    private:
        size_t m_pendingCount{ 0 };
        std::mutex m_ready_lock{};

        std::atomic<size_t> m_runningCount;

        MetaData m_meta;
        SkyboxData m_skybox;

        RootData m_root;
        ScriptEngineData m_scriptEngineData;

        std::vector<EntityData> m_entities;

        std::vector<FontData> m_fonts;

        Material m_defaultMaterial;
        std::vector<MaterialData> m_materials;

        std::vector<SpriteData> m_sprites;

        RootLoader m_rootLoader;

        ScriptLoader m_scriptLoader;

        SkyboxLoader m_skyboxLoader;
        VolumeLoader m_volumeLoader;
        CubeMapLoader m_cubeMapLoader;

        EntityLoader m_entityLoader;

        FontLoader m_fontLoader;
        MaterialLoader m_materialLoader;
        CustomMaterialLoader m_customMaterialLoader;
        SpriteLoader m_spriteLoader;

        CameraLoader m_cameraLoader;
        LightLoader m_lightLoader;
        ControllerLoader m_controllerLoader;
        AudioLoader m_audioLoader;
        GeneratorLoader m_generatorLoader;
        PhysicsLoader m_physicsLoader;
    };
}
