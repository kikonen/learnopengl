#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <mutex>

#include <fmt/format.h>

#include "ki/size.h"
#include "ki/uuid.h"

#include "backend/DrawOptions.h"

#include "model/Node.h"

#include "mesh/MeshType.h"

#include "component/NodeComponent.h"

class Registry;

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

    void attachListeners();

    //inline bool containsNode(const int id) const noexcept
    //{
    //    const auto& it = m_idToNode.find(id);
    //    return it != m_idToNode.end();
    //}

    // @return node null if not found
    inline Node* getNode(const ki::node_id id) const noexcept
    {
        const auto& it = m_idToNode.find(id);
        return it != m_idToNode.end() ? it->second : nullptr;
    }

    // @return node null if not found
    inline Node* getNode(const uuids::uuid& id) const noexcept
    {
        const auto& it = m_uuidToNode.find(id);
        return it != m_uuidToNode.end() ? it->second : nullptr;
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
        const uuids::uuid& parentId) noexcept;

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

    inline const Material& getSelectionMaterial() const noexcept {
        return m_selectionMaterial;
    }

    void setSelectionMaterial(const Material& material) {
        m_selectionMaterial = material;
    }

private:
    void setActiveNode(Node* node);
    void setActiveCameraNode(Node* node);

    void attachNode(
        Node* node,
        const uuids::uuid& uuid,
        const uuids::uuid& parentUUID,
        ki::node_id parentId) noexcept;

    void bindPendingChildren();

    void bindNode(
        const uuids::uuid& uuid,
        Node* node);

    bool bindParent(
        Node* node,
        const uuids::uuid& uuid,
        const uuids::uuid& parentUUID,
        ki::node_id parentId);

    void bindChildren(
        const uuids::uuid& parentUUID);

    void bindSkybox(
        Node* node) noexcept;

public:
    // EntityRegistry
    std::vector<Node*> m_allNodes;

    Node* m_root{ nullptr };

    Node* m_skybox{ nullptr };

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    Registry* m_registry{ nullptr };

    std::mutex m_lock{};

    std::unordered_map<ki::node_id, Node*> m_idToNode;
    std::unordered_map<uuids::uuid, Node*> m_uuidToNode;

    std::unordered_map<uuids::uuid, std::vector<std::tuple<const uuids::uuid, Node*>>> m_pendingChildren;

    //std::unordered_map<ki::node_id, NodeVector> m_parentToChildren;

    std::vector<NodeComponent<Camera>> m_cameraComponents;

    NodeVector m_cameraNodes;

    NodeVector m_dirLightNodes;
    NodeVector m_pointLightNodes;
    NodeVector m_spotLightNodes;

    Node* m_activeNode{ nullptr };
    Node* m_activeCameraNode{ nullptr };

    Material m_selectionMaterial;

    mutable int m_taggedCount{ -1 };
    mutable int m_selectedCount{ -1 };
};
