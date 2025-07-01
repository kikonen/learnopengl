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
    class Loaders;
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
            const ki::node_id parentId,
            const NodeData& baseData,
            std::shared_ptr<Context> ctx,
            std::shared_ptr<Loaders> loaders);

    private:
        SceneLoader* const m_sceneLoader;

        std::mutex m_resolvedNodesLock{};
        std::vector<ResolvedNode> m_resolvedNodes;
    };
}
