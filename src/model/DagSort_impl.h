#include "DagSort.h"

#include <vector>
#include <map>

#include "ki/size.h"

namespace dag {
    enum class Color {
        white,
        gray,
        black,
    };

    template <typename T, class U>
    struct DagNode {
        DagItem<T, U>* item;

        DagNode* previous{ nullptr };
        int depth{ 0 };
        int time{ 0 };
        Color color{ Color::white };

        std::vector<DagNode*> children;
    };

    template <typename T, class U>
    void createNodes(
        std::vector<DagItem<T, U>>& items,
        std::vector<DagNode<T, U>>& nodes)
    {
        nodes.reserve(items.size());

        std::map<ki::node_id, std::vector<ki::node_id>> parentToChildren;

        for (auto& resolved : items) {
            parentToChildren[resolved.parentId].push_back(resolved.nodeId);
        }

        for (auto& item : items) {
            auto nodeId = item.nodeId;

            auto& node = nodes.emplace_back();
            node.item = &item;
        }

        std::map<ki::node_id, DagNode<T, U>*> idToNode;
        for (auto& node : nodes) {
            idToNode.insert({ node.item->nodeId, &node });
        }

        for (auto& node : nodes) {
            const auto& it = parentToChildren.find(node.item->nodeId);
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

    template <typename T, class U>
    void visit(
        DagNode<T, U>* node,
        int& time,
        std::vector<DagNode<T, U>*>& result)
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

    template <typename T, class U>
    void dfs(
        std::vector<DagNode<T, U>>& nodes,
        std::vector<DagNode<T, U>*>& result)
    {
        int time = 0;
        for (auto& node : nodes) {
            if (node.color == Color::white) {
                visit(&node, time, result);
            }
        }
    }

    template <typename T, class U>
    std::vector<DagItem<T, U>> DagSort<T, U>::sort(
        std::vector<DagItem<T, U>>& items)
    {
        std::vector<DagNode<T, U>> nodes;
        std::vector<DagNode<T, U>*> stack;

        createNodes(items, nodes);
        dfs(nodes, stack);

        std::reverse(stack.begin(), stack.end());

        std::vector<DagItem<T, U>> result;
        for (auto* node : stack) {
            result.push_back(*node->item);
        }

        return result;
    }
}
