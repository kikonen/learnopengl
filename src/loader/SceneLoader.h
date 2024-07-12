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
    struct NodeHandle;
    struct TypeHandle;
}

namespace mesh {
    class MeshSet;
    class MeshType;
    struct LodMesh;
}

namespace loader {
    struct RootData;
    struct SkyboxData;
    struct MaterialData;
    struct ScriptEngineData;
    struct NodeRoot;
    struct NodeData;
    struct TextData;
    struct MeshData;
    struct LodData;
    struct AnimationData;
    struct ResolvedNode;
    struct MaterialData;

    struct FlagContainer;

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
        void loadedNode(
            const NodeRoot& nodeRoot,
            bool success);

        void attach(
            const RootData& root);

        void notifySceneLoaded();

        void attachResolvedNodes(
            std::vector<ResolvedNode>& resolvedNodes);

        void attachResolvedNode(
            const ResolvedNode& resolvedNode);

        void addResolvedNode(
            const ResolvedNode& resolvedNode);

        bool resolveNode(
            const ki::node_id rootId,
            const NodeRoot& nodeRoot);

        pool::TypeHandle resolveNodeClone(
            pool::TypeHandle typeHandle,
            const ki::node_id rootId,
            const NodeRoot& nodeRoot,
            const NodeData& nodeData,
            bool cloned,
            int cloneIndex);

        pool::TypeHandle resolveNodeCloneRepeat(
            pool::TypeHandle typeHandle,
            const ki::node_id rootId,
            const NodeRoot& nodeRoot,
            const NodeData& nodeData,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset);

        void assignTypeFlags(
            const NodeData& nodeData,
            pool::TypeHandle typeHandle);

        void assignMeshFlags(
            const FlagContainer& container,
            mesh::LodMesh& lodMesh);

        void assignNodeFlags(
            const FlagContainer& container,
            pool::NodeHandle nodeHandle);

        const pool::TypeHandle createType(
            const NodeData& nodeData,
            const glm::uvec3& tile);

        void resolveLodLevels(
            mesh::MeshType* type,
            const NodeData& nodeData);

        void resolveMaterials(
            mesh::MeshType* type,
            mesh::LodMesh& lodMesh,
            const NodeData& nodeData,
            const MeshData& meshData);

        void resolveMeshes(
            mesh::MeshType* type,
            const NodeData& nodeData,
            const glm::uvec3& tile);

        void resolveMesh(
            mesh::MeshType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            const glm::uvec3& tile,
            int index);

        const LodData* resolveLod(
            mesh::MeshType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        void resolveSockets(
            const MeshData& meshData,
            mesh::MeshSet& meshSet);

        void loadAnimation(
            const std::string& baseDir,
            const AnimationData& data,
            mesh::MeshSet& meshSet);

        pool::NodeHandle createNode(
            pool::TypeHandle typeHandle,
            const ki::node_id rootId,
            const NodeData& nodeData,
            const bool cloned,
            const int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& clonePositionOffset,
            const glm::vec3& tilePositionOffset);

        void resolveAttachments(
            pool::TypeHandle typeHandle,
            pool::NodeHandle nodeHandle,
            const NodeData& nodeData);

        void loadMeta(
            const loader::DocNode& node,
            MetaData& data) const;

        void validate(
            const RootData& root);

        void validateNode(
            const ki::node_id rootId,
            const NodeRoot& nodeRoot,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateNodeClone(
            const ki::node_id rootId,
            const NodeRoot& nodeRoot,
            const NodeData& nodeData,
            bool cloned,
            int cloneIndex,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateNodeCloneRepeat(
            const ki::node_id rootId,
            const NodeRoot& nodeRoot,
            const NodeData& nodeData,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

    private:
        size_t m_pendingCount{ 0 };
        std::mutex m_ready_lock{};

        std::atomic<size_t> m_runningCount;

        std::unique_ptr<MetaData> m_meta;
        std::unique_ptr<SkyboxData> m_skybox;

        std::unique_ptr<RootData> m_root;
        std::unique_ptr<ScriptEngineData> m_scriptEngineData;

        std::vector<NodeRoot> m_nodes;

        std::vector<ResolvedNode> m_resolvedNodes;

        std::unique_ptr<Material> m_defaultMaterial;

        std::unique_ptr<Loaders> m_loaders;
    };
}
