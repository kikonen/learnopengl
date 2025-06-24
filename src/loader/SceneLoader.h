#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include "BaseLoader.h"

struct Material;
class Registry;

namespace pool {
    struct NodeHandle;
    struct TypeHandle;
}

namespace loader {
    struct RootData;
    struct SkyboxData;
    struct ScriptSystemData;
    struct NodeTypeData;
    struct NodeData;
    struct ResolvedNode;
    struct DecalData;
    struct MaterialUpdaterData;

    class NodeTypeBuilder;
    class NodeBuilder;

    struct MetaData {
        std::string name;

        //std::string assetsDir;
        //std::string modelsDir;
    };

    class SceneLoader : public BaseLoader
    {
    public:
        SceneLoader(
            std::shared_ptr<Context> ctx);

        ~SceneLoader();

        void destroy();

        bool isRunning();

        void prepare(
            std::shared_ptr<Registry> registry);

        void load();

        void loadedNode(
            const NodeData& nodeData,
            bool success);

    private:
        void attach(
            const RootData& root);

        void notifySceneLoaded();

        void attachResolvedNodes(
            std::vector<ResolvedNode>& resolvedNodes);

        void attachResolvedNode(
            const ResolvedNode& resolvedNode);

        void loadMeta(
            const loader::DocNode& node,
            MetaData& data) const;

        void validate(
            const RootData& root);

        void validateNode(
            const ki::node_id rootId,
            const NodeData& node,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateNodeClone(
            const ki::node_id rootId,
            const NodeData& baseData,
            const NodeData& cloneData,
            bool cloned,
            int cloneIndex,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateNodeCloneRepeat(
            const ki::node_id rootId,
            const NodeData& baseData,
            const NodeData& cloneData,
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
        std::unique_ptr<ScriptSystemData> m_scriptSystemData;

        std::vector<NodeTypeData> m_nodeTypes;
        std::vector<NodeData> m_nodes;

        std::unique_ptr<Material> m_defaultMaterial;

        std::vector<DecalData> m_decals;
        std::vector<MaterialUpdaterData> m_materialUpdaters;

        std::shared_ptr<Loaders> m_loaders;

        std::unique_ptr<NodeTypeBuilder> m_nodeTypeBuilder;
        std::unique_ptr<NodeBuilder> m_nodeBuilder;
    };
}
