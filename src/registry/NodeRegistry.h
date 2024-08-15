#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
//#include <mutex>
#include <functional>

#include <fmt/format.h>

#include "ki/size.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "component/CameraComponent.h"
#include "component/NodeComponent.h"

struct Material;

struct UpdateContext;

class Registry;
class NodeSnapshotRegistry;


class NodeRegistry final
{
    friend class Node;

public:
    static NodeRegistry& get() noexcept;

    NodeRegistry();
    NodeRegistry& operator=(const NodeRegistry&) = delete;

    ~NodeRegistry();

    void prepare(
        Registry* registry);

    void updateWT(const UpdateContext& ctx);

    void updateModelMatrices();
    void postUpdateWT();

    void updateRT(const UpdateContext& ctx);
    void updateEntity(const UpdateContext& ctx);

    void snapshotWT(NodeSnapshotRegistry& snapshotRegistry);

    void attachListeners();

    void handleNodeAdded(Node* node);

    // @return root if root is prepared for RT
    Node* getRootRT() const noexcept {
        return m_rootRT.toNode();
    }

    // @return root if root is prepared for WT
    Node* getRootWT() const noexcept {
        return m_rootWT.toNode();
    }

    void selectNode(pool::NodeHandle handle, bool append) const noexcept;

    int countTagged() const noexcept;
    int countSelected() const noexcept;

    void clearTaggedCount() const noexcept {
        m_taggedCount = -1;
    }
    void clearSelectedCount() const noexcept {
        m_selectedCount = -1;
    }

    void changeParent(
        const ki::node_id id,
        const ki::node_id parentId) noexcept;

    inline Node* getActiveNode() const noexcept { return m_activeNode.toNode(); }
    inline Node* getActiveCameraNode() const noexcept { return m_activeCameraNode.toNode(); }

    uint32_t getNodeCount() const noexcept
    {
        return static_cast<uint32_t>(m_allNodes.size());
    }

    const std::vector<Node*> getCachedNodesRT() const noexcept
    {
        return m_cachedNodesRT;
    }

    pool::NodeHandle getNextCameraNode(
        pool::NodeHandle srcNode,
        int offset) const noexcept;

    pool::NodeHandle findDefaultCameraNode() const;

    const pool::NodeHandle& getDirLightNode() const noexcept {
        return m_dirLightNodes.empty() ? pool::NodeHandle::NULL_HANDLE : m_dirLightNodes[0];
    }

    const std::vector<pool::NodeHandle>& getPointLightNodes() const noexcept {
        return m_pointLightNodes;
    }

    const std::vector<pool::NodeHandle>& getSpotLightNodes() const noexcept {
        return m_spotLightNodes;
    }

    const Material& getSelectionMaterial() const noexcept;
    void setSelectionMaterial(const Material& material);

    //ki::level_id getLevel() const noexcept {
    //    std::lock_guard lock(m_lock);
    //    return m_nodeLevel;
    //}

    //void withLock(const std::function<void(NodeRegistry&)>& fn);

private:
    void setActiveNode(pool::NodeHandle node);
    void setActiveCameraNode(pool::NodeHandle node);

    void attachNode(
        const ki::node_id nodeId,
        const ki::node_id parentId) noexcept;

    void bindNode(
        const ki::node_id nodeId);

    bool bindParent(
        const ki::node_id nodeId,
        const ki::node_id parentId);

    void bindSkybox(
        pool::NodeHandle handle) noexcept;

public:
    pool::NodeHandle m_skybox{};

private:
    pool::NodeHandle m_rootWT{};
    pool::NodeHandle m_rootRT{};
    bool m_rootPreparedRT{ false };

    // Internal tracking
    std::vector<pool::NodeHandle> m_allNodes;

    ki::level_id m_nodeLevel{ 0 };
    ki::level_id m_cachedNodeLevel{ 0 };

    std::vector<Node*> m_cachedNodesWT;
    std::vector<Node*> m_cachedNodesRT;

    Registry* m_registry{ nullptr };

    //mutable std::mutex m_lock;

    //std::vector<NodeComponent<CameraComponent>> m_cameraComponents;

    std::vector<pool::NodeHandle> m_cameraNodes;

    std::vector<pool::NodeHandle> m_dirLightNodes;
    std::vector<pool::NodeHandle> m_pointLightNodes;
    std::vector<pool::NodeHandle> m_spotLightNodes;

    pool::NodeHandle m_activeNode{};
    pool::NodeHandle m_activeCameraNode{};

    std::unique_ptr<Material> m_selectionMaterial;

    mutable int m_taggedCount{ -1 };
    mutable int m_selectedCount{ -1 };
};
