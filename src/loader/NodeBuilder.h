#pragma once

#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <atomic>

#include <glm/glm.hpp>

#include "ki/size.h"

struct CreateState;

namespace pool {
    struct NodeHandle;
    struct TypeHandle;
}

namespace loader {
    struct Context;
    class Loaders;
    class SceneLoader;

    class NodeTypeBuilder;

    struct RootData;
    struct NodeData;
    struct ResolvedNode;

    struct FlagContainer;

    class NodeBuilder {
    public:
        NodeBuilder(
            SceneLoader* sceneLoader,
            std::shared_ptr<Context> ctx,
            std::shared_ptr<Loaders> loaders);

        ~NodeBuilder();

        void loadedNode(
            const NodeData& baseData,
            bool success);

        std::vector<ResolvedNode>& getResolvedNodes()
        {
            std::lock_guard lock(m_resolvedNodesLock);
            return m_resolvedNodes;
        }

        void addResolvedNode(
            const ResolvedNode& resolved);

        bool resolveNode(
            const ki::node_id ownerId,
            const NodeData& baseData);

    private:

        void resolveNode(
            const ki::node_id ownerId,
            const NodeData& baseData,
            bool root);

        void resolveNodeClone(
            const ki::node_id ownerId,
            const NodeData& nodeData,
            bool cloned,
            int cloneIndex);

        void resolveNodeCloneRepeat(
            const ki::node_id ownerId,
            const NodeData& nodeData,
            bool cloned,
            int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& tilePositionOffset);

        std::pair<pool::NodeHandle, CreateState> createNode(
            const NodeData& nodeData,
            const int cloneIndex,
            const glm::uvec3& tile,
            const glm::vec3& positionOffset);

    private:
        SceneLoader* const m_sceneLoader;

        std::mutex m_resolvedNodesLock{};
        std::vector<ResolvedNode> m_resolvedNodes;

        std::shared_ptr<Context> m_ctx;
        std::shared_ptr<Loaders> m_loaders;
    };
}
