#pragma once

#include <vector>

#include <imgui.h>

#include "model/Node.h"

namespace editor
{
    class NodeTool;

    struct NodeTreeNode;
    struct NodeTreeNode
    {
        const model::Node* const m_node;
        std::vector<std::unique_ptr<NodeTreeNode>> m_children;

        NodeTreeNode(const model::Node* node)
            : m_node{ node }
        {
        }
    };

    struct NodeTree
    {
        editor::NodeTool& m_tool;
        std::unique_ptr<NodeTreeNode> m_root;

        NodeTree(editor::NodeTool& tool)
            : m_tool{ tool }
        {
        }

        void build(const std::vector<model::Node*> nodes)
        {
            std::unordered_map<uint32_t, NodeTreeNode*> collectedNodes;

            const auto& nodeRegistry = NodeRegistry::get();

            for (const auto* node : nodes) {
                // NOTE KI skip null & identity entity
                if (!node) continue;

                // NOTE KI first node in list must be root
                if (!m_root) {
                    m_root = std::make_unique<NodeTreeNode>(node);
                    collectedNodes.insert({ node->getEntityIndex(), m_root.get() });
                    continue;
                }

                auto parentIndex = nodeRegistry.getParentIndex(node->getEntityIndex());

                const auto& it = collectedNodes.find(parentIndex);
                auto* parentTree = it == collectedNodes.end() ? nullptr : it->second;

                auto& treeNode = parentTree->m_children.emplace_back(std::make_unique<NodeTreeNode>(node));
                collectedNodes.insert({ node->getEntityIndex(), treeNode.get() });
            }
        }

        void drawNode(
            const gui::FrameContext& ctx,
            NodeTreeNode* curr,
            bool defaultExpanded)
        {
            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None;
            flags |= ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            flags |= ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
            flags |= ImGuiTreeNodeFlags_SpanFullWidth;

            if (defaultExpanded) {
                flags |= ImGuiTreeNodeFlags_DefaultOpen;
            }
            if (curr->m_children.empty()) {
                flags |= ImGuiTreeNodeFlags_Leaf;
            }

            const auto currNode = m_tool.m_state.m_selectedNode.toNode();

            const auto* node = curr->m_node;
            const auto* nodeName = node->getName().c_str();

            if (node == currNode) {
                flags |= ImGuiTreeNodeFlags_Selected;
            }

            if (ImGui::TreeNodeEx(nodeName, flags))
            {
                if (ImGui::IsItemClicked()) {
                    m_tool.onSelectNode(ctx, node->toHandle());
                }

                for (auto& child : curr->m_children) {
                    drawNode(ctx, child.get(), false);
                }
                ImGui::TreePop();
            }
        }
    };
}
