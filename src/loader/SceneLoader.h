#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include "BaseLoader.h"

namespace model
{
    struct ResolvedNode;
}

class Registry;

namespace pool {
    struct NodeHandle;
    struct TypeHandle;
}

namespace loader {
    struct SceneData;
    struct MetaData;
    struct RootData;
    struct SkyboxData;
    struct ScriptSystemData;
    struct NodeTypeData;
    struct NodeData;
    struct CompositeData;
    struct DecalData;
    struct ParticleData;
    struct MaterialUpdaterData;

    class NodeTypeBuilder;
    class NodeBuilder;

    class SceneLoader : public BaseLoader
    {
    public:
        SceneLoader(
            std::shared_ptr<Context> ctx);

        ~SceneLoader();

        void destroy();

        bool isRunning();

        void prepare(
            Registry* registry);

        void load();

        void loadedNode(
            const NodeData& nodeData,
            bool success);

    private:
        void attach(
            const RootData& root);

        void notifySceneLoaded();

        void attachResolvedNodes(
            std::vector<model::ResolvedNode>& resolvedNodes);

        void attachResolvedNode(
            const model::ResolvedNode& resolvedNode);

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
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

        void validateNodeCloneRepeat(
            const ki::node_id rootId,
            const NodeData& baseData,
            const NodeData& cloneData,
            bool cloned,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset,
            int pass,
            int& errorCount,
            std::map<ki::node_id, std::string>& collectedIds);

    private:
        size_t m_pendingCount{ 0 };
        std::mutex m_ready_lock{};

        std::unique_ptr<SceneData> m_sceneData;

        std::shared_ptr<Loaders> m_loaders;

        std::unique_ptr<NodeTypeBuilder> m_nodeTypeBuilder;
        std::unique_ptr<NodeBuilder> m_nodeBuilder;
    };
}
