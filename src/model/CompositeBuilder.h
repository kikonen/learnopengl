#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

namespace model
{
    class NodeType;
    struct CreateState;
    struct CompositeDefinition;
    struct NodeDefinition;
    struct ResolvedNode;
}

class Registry;
class NodeRegistry;

namespace pool {
    struct NodeHandle;
    struct TypeHandle;
}

namespace model
{
    class CompositeBuilder {
    public:
        CompositeBuilder(
            NodeRegistry& nodeRegistry);

        ~CompositeBuilder();

        const std::vector<ResolvedNode>& getResolvedNodes() const noexcept
        {
            return m_resolvedNodes;
        }

        pool::NodeHandle build(
            const ki::node_id parentId,
            const ki::socket_id socketId,
            const model::NodeType* type,
            const CreateState& state);

        bool build(
            const ki::node_id parentId,
            const CompositeDefinition& baseData,
            std::vector<std::pair<std::string, ki::node_id>> aliases);

        void buildNode(
            const ki::node_id parentId,
            const NodeDefinition& baseData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases,
            bool root);

        pool::NodeHandle attach();
        pool::NodeHandle asyncAttach(Registry* registry);
        pool::NodeHandle asyncAttach(
            const ResolvedNode& resolved,
            Registry* registry);

    private:
        void addResolvedNode(
            const ResolvedNode& resolved);

        void buildNodeClone(
            const ki::node_id parentId,
            const NodeDefinition& nodeData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases);

        void buildNodeCloneRepeat(
            const ki::node_id parentId,
            const NodeDefinition& nodeData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases,
            const glm::vec3& tilePositionOffset);

        std::pair<pool::NodeHandle, CreateState> createNode(
            const NodeDefinition& nodeData,
            std::vector<std::pair<std::string, ki::node_id>>& aliases,
            const glm::vec3& positionOffset);

    private:
        NodeRegistry& m_nodeRegistry;

        std::vector<ResolvedNode> m_resolvedNodes;
    };
}
