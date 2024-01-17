#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <mutex>

#include <fmt/format.h>

#include "ki/size.h"

#include "component/Camera.h"
#include "component/NodeComponent.h"

class Assets;
struct Material;

struct UpdateContext;

class Registry;
class SnapshotRegistry;

class Node;
using NodeVector = std::vector<Node*>;


class NodeRegistry final
{
public:
    NodeRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~NodeRegistry();

    void prepare(
        Registry* registry);

    void updateWT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);
    void updateEntity(const UpdateContext& ctx);

    void snapshotWT(SnapshotRegistry& snapshotRegistry);
    void snapshotRT(SnapshotRegistry& snapshotRegistry);

    void attachListeners();

    void handleNodeAdded(Node* node);

    //inline bool containsNode(const int id) const noexcept
    //{
    //    const auto& it = m_idToNode.find(id);
    //    return it != m_idToNode.end();
    //}

    // @return root if root is prepared for RT
    Node* getRootRT() const noexcept {
        return m_rootPreparedRT ? m_root : nullptr;
    }

    // @return root if root is prepared for WT
    Node* getRootWT() const noexcept {
        return m_root;
    }

    // @return node null if not found
    inline Node* getNode(const ki::node_id id) const noexcept
    {
        const auto& it = m_idToNode.find(id);
        return it != m_idToNode.end() ? it->second : nullptr;
    }

    void selectNodeById(ki::node_id id, bool append) const noexcept;

    int countTagged() const noexcept;
    int countSelected() const noexcept;

    void clearTaggedCount() const noexcept {
        m_taggedCount = -1;
    }
    void clearSelectedCount() const noexcept {
        m_selectedCount = -1;
    }

    void changeParent(
        Node* node,
        const ki::node_id parentId) noexcept;

    //inline const NodeVector* getChildren(const Node& parent) const noexcept {
    //    const auto& it = m_parentToChildren.find(parent.m_id);
    //    return it != m_parentToChildren.end() ? &it->second : nullptr;
    //}

    inline Node* getActiveNode() const noexcept { return m_activeNode; }
    inline Node* getActiveCameraNode() const noexcept { return m_activeCameraNode; }

    Node* getNextCameraNode(Node* srcNode, int offset) const noexcept;

    Node* findDefaultCameraNode() const;

    const Node* getDirLightNode() const noexcept {
        return m_dirLightNodes.empty() ? nullptr : m_dirLightNodes[0];
    }

    const NodeVector& getPointLightNodes() const noexcept {
        return m_pointLightNodes;
    }

    const NodeVector& getSpotLightNodes() const noexcept {
        return m_spotLightNodes;
    }

    const Material& getSelectionMaterial() const noexcept;
    void setSelectionMaterial(const Material& material);

private:
    void setActiveNode(Node* node);
    void setActiveCameraNode(Node* node);

    void attachNode(
        Node* node,
        const ki::node_id nodeId,
        const ki::node_id parentId) noexcept;

    void bindPendingChildren();

    void bindNode(
        const ki::node_id nodeId,
        Node* node);

    bool bindParent(
        Node* node,
        const ki::node_id nodeId,
        const ki::node_id parentId);

    void bindChildren(
        const ki::node_id parentId);

    void bindSkybox(
        Node* node) noexcept;

public:
    Node* m_skybox{ nullptr };

private:
    const Assets& m_assets;

    Node* m_root{ nullptr };
    bool m_rootPreparedRT{ false };

    // EntityRegistry
    std::vector<Node*> m_allNodes;

    std::shared_ptr<std::atomic<bool>> m_alive;

    Registry* m_registry{ nullptr };

    mutable std::mutex m_snapshotLock{};

    std::unordered_map<ki::node_id, Node*> m_idToNode;

    std::unordered_map<ki::node_id, std::vector<std::tuple<const ki::node_id, Node*>>> m_pendingChildren;

    //std::unordered_map<ki::node_id, NodeVector> m_parentToChildren;

    std::vector<NodeComponent<Camera>> m_cameraComponents;

    NodeVector m_cameraNodes;

    NodeVector m_dirLightNodes;
    NodeVector m_pointLightNodes;
    NodeVector m_spotLightNodes;

    Node* m_activeNode{ nullptr };
    Node* m_activeCameraNode{ nullptr };

    std::unique_ptr<Material> m_selectionMaterial;

    mutable int m_taggedCount{ -1 };
    mutable int m_selectedCount{ -1 };
};
