#pragma once

#include <string>
#include <vector>
#include <memory>

#include <glm/glm.hpp>

#include "ki/size.h"

class NodeRegistry;
struct CreateState;
struct CompositeDefinition;
struct NodeDefinition;
struct ResolvedNode;

namespace pool {
    struct NodeHandle;
    struct TypeHandle;
}

class CompositeBuilder {
public:
    CompositeBuilder(
        NodeRegistry& nodeRegistry);

    ~CompositeBuilder();

    std::vector<ResolvedNode>& getResolvedNodes()
    {
        return m_resolvedNodes;
    }

    void addResolvedNode(
        const ResolvedNode& resolved);

    bool build(
        const ki::node_id parentId,
        const CompositeDefinition& baseData);

private:
    void buildNode(
        const ki::node_id parentId,
        const NodeDefinition& baseData,
        std::vector<std::pair<std::string, ki::node_id>>& aliases,
        bool root);

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
