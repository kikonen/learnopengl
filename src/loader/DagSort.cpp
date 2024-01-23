#include "DagSort.h"

#include <vector>
#include <map>

#include "ki/size.h"

namespace {
    enum class Color {
        white,
        gray,
        black,
    };

    struct DagNode {
        ki::node_id id;
        loader::ResolvedEntity* resolved;

        DagNode* previous{ nullptr };
        int depth{ 0 };
        int time{ 0 };
        Color color{ Color::white };

        std::vector<DagNode*> children;
    };

    void createNodes(
        std::vector<loader::ResolvedEntity>& resolvedEntities,
        std::vector<DagNode>& nodes)
    {
        nodes.reserve(resolvedEntities.size());

        std::map<ki::node_id, std::vector<ki::node_id>> parentToChildren;

        for (auto& resolved : resolvedEntities) {
            parentToChildren[resolved.parentId].push_back(resolved.handle.toId());
        }

        for (auto& resolved : resolvedEntities) {
            auto nodeId = resolved.handle.toId();

            auto& node = nodes.emplace_back();
            node.id = nodeId;
            node.resolved = &resolved;
        }

        std::map<ki::node_id, DagNode*> idToNode;
        for (auto& node : nodes) {
            idToNode.insert({ node.id, &node });
        }

        for (auto& node : nodes) {
            const auto& it = parentToChildren.find(node.id);
            if (it != parentToChildren.end()) {
                for (auto& childId : it->second) {
                    auto it = idToNode.find(childId);
                    if (it != idToNode.end()) {
                        node.children.push_back(it->second);
                    }
                }
            }
        }
    }

    void visit(
        DagNode* node,
        int& time,
        std::vector<DagNode*>& result)
    {
        time++;
        node->depth = time;
        node->color = Color::gray;
        for (auto& child : node->children) {
            if (child->color == Color::white) {
                child->previous = node;
                visit(child, time, result);
            }
        }
        node->color = Color::black;
        time++;
        node->time = time;
        result.push_back(node);
    }

    void dfs(
        std::vector<DagNode>& nodes,
        std::vector<DagNode*>& result)
    {
        int time = 0;
        for (auto& node : nodes) {
            if (node.color == Color::white) {
                visit(&node, time, result);
            }
        }
    }
}

namespace loader {
    std::vector<ResolvedEntity*> DagSort::sort(std::vector<ResolvedEntity>& resolvedEntities)
    {
        std::vector<DagNode> nodes;
        std::vector<DagNode*> stack;

        createNodes(resolvedEntities, nodes);
        dfs(nodes, stack);

        std::reverse(stack.begin(), stack.end());

        std::vector<ResolvedEntity*> result;
        for (auto* node : stack) {
            result.push_back(node->resolved);
        }

        return result;
    }
}
