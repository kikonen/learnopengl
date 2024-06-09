#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include "text/size.h"

#include "BaseLoader.h"

struct Material;
class Registry;

namespace text {
}

namespace pool {
    class NodeHandle;
    class TypeHandle;
}

namespace mesh {
    class MeshSet;
    class MeshType;
    struct LodMesh;
}

namespace loader {
    struct RootData;
    struct SkyboxData;
    struct FontData;
    struct MaterialData;
    struct ScriptEngineData;
    struct EntityRoot;
    struct EntityData;
    struct TextData;
    struct MeshData;
    struct LodData;
    struct AnimationData;
    struct ResolvedEntity;
    struct MaterialData;

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
            const EntityRoot& entityRoot,
            bool success);

        void attach(
            const RootData& root);

        void notifySceneLoaded();

        void attachResolvedEntities(
            std::vector<ResolvedEntity>& resolvedEntities);

        void attachResolvedEntity(
            const ResolvedEntity& resolvedEntity);

        void addResolvedEntity(
            const ResolvedEntity& resolvedEntity);

        bool resolveEntity(
            const ki::node_id rootId,
            const EntityRoot& entityRoot);

        pool::TypeHandle resolveEntityClone(
            pool::TypeHandle typeHandle,
            const ki::node_id rootId,
            const EntityRoot& entityRoot,
            const EntityData& entityData,
            bool cloned,
            int cloneIndex);

        pool::TypeHandle resolveEntityCloneRepeat(
            pool::TypeHandle typeHandle,
            const ki::node_id rootId,
            const EntityRoot& entityRoot,
            const EntityData& entityData,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset);

        void assignTypeFlags(
            const EntityData& entityData,
            pool::TypeHandle typeHandle);

        void assignMeshFlags(
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        void assignNodeFlags(
            const EntityData& entityData,
            pool::NodeHandle nodeHandle);

        const pool::TypeHandle createType(
            const EntityData& entityData,
            const glm::uvec3& tile);

        void resolveProgram(
            mesh::MeshType* type,
            mesh::LodMesh& lodMesh,
            const EntityData& entityData);

        text::font_id resolveFont(
            pool::TypeHandle typeHandle,
            const TextData& data) const;

        void resolveMaterials(
            mesh::MeshType* type,
            mesh::LodMesh& lodMesh,
            const EntityData& entityData,
            const MeshData& meshData);

        void resolveMeshes(
            mesh::MeshType* type,
            const EntityData& entityData,
            const glm::uvec3& tile);

        void resolveMesh(
            mesh::MeshType* type,
            const EntityData& entityData,
            const MeshData& meshData,
            const glm::uvec3& tile,
            int index);

        void resolveLods(
            mesh::MeshType* type,
            const EntityData& entityData);

        void resolveLod(
            mesh::MeshType* type,
            const LodData& lodData);

        void loadAnimation(
            const std::string& baseDir,
            const AnimationData& data,
            mesh::MeshSet& meshSet);

        pool::NodeHandle createNode(
            pool::TypeHandle typeHandle,
            const ki::node_id rootId,
            const EntityData& entityData,
            const bool cloned,
            const int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& clonePositionOffset,
            const glm::vec3& tilePositionOffset);

        void loadMeta(
            const loader::DocNode& node,
            MetaData& data) const;

        void validate(
            const RootData& root);

        void validateEntity(
            const ki::node_id rootId,
            const EntityRoot& entityRoot,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateEntityClone(
            const ki::node_id rootId,
            const EntityRoot& entityRoot,
            const EntityData& entityData,
            bool cloned,
            int cloneIndex,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateEntityCloneRepeat(
            const ki::node_id rootId,
            const EntityRoot& entityRoot,
            const EntityData& entityData,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        const FontData* findFont(
            std::string_view name) const;

    private:
        size_t m_pendingCount{ 0 };
        std::mutex m_ready_lock{};

        std::atomic<size_t> m_runningCount;

        std::unique_ptr<MetaData> m_meta;
        std::unique_ptr<SkyboxData> m_skybox;

        std::unique_ptr<RootData> m_root;
        std::unique_ptr<ScriptEngineData> m_scriptEngineData;

        std::vector<EntityRoot> m_entities;

        std::vector<ResolvedEntity> m_resolvedEntities;

        std::vector<FontData> m_fonts;

        std::unique_ptr<Material> m_defaultMaterial;

        std::unique_ptr<Loaders> m_loaders;
    };
}
