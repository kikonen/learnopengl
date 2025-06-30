#pragma once

#include <string>
#include <vector>
#include <memory>
#include <mutex>

#include <glm/glm.hpp>

#include "ki/size.h"

struct CreateState;
struct ResolvedNode;

namespace pool {
    struct NodeHandle;
    struct TypeHandle;
}

namespace loader {
    class SceneLoader;
    struct Context;
    struct NodeData;

    class NodeBuilder {
    public:
        NodeBuilder(
            SceneLoader* sceneLoader);

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
            const NodeData& baseData,
            std::shared_ptr<Context> ctx);

    private:

        void resolveNode(
            const ki::node_id ownerId,
            const NodeData& baseData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases,
            bool root);

        void resolveNodeClone(
            const ki::node_id ownerId,
            const NodeData& nodeData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases);

        void resolveNodeCloneRepeat(
            const ki::node_id ownerId,
            const NodeData& nodeData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases,
            const glm::vec3& tilePositionOffset);

        std::pair<pool::NodeHandle, CreateState> createNode(
            const NodeData& nodeData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases,
            const glm::vec3& positionOffset);

    private:
        SceneLoader* const m_sceneLoader;

        std::mutex m_resolvedNodesLock{};
        std::vector<ResolvedNode> m_resolvedNodes;
    };
}
