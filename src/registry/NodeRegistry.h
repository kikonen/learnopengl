#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <mutex>

#include <fmt/format.h>

#include "ki/size.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "component/Camera.h"
#include "component/NodeComponent.h"

struct Material;

struct UpdateContext;

class Registry;
class SnapshotRegistry;


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
    void updateRT(const UpdateContext& ctx);
    void updateEntity(const UpdateContext& ctx);

    void snapshotWT(SnapshotRegistry& snapshotRegistry);
    void snapshotRT(SnapshotRegistry& snapshotRegistry);

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
        const ki::node_id id,
        const ki::node_id parentId) noexcept;

    inline Node* getActiveNode() const noexcept { return m_activeNode.toNode(); }
    inline Node* getActiveCameraNode() const noexcept { return m_activeCameraNode.toNode(); }

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

private:
    void cacheNodes(std::vector<Node*>& cache) const noexcept;

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

    // EntityRegistry
    std::vector<pool::NodeHandle> m_allNodes;

    std::vector<Node*> m_cachedNodesWT;
    std::vector<Node*> m_cachedNodesRT;

    Registry* m_registry{ nullptr };

    mutable std::mutex m_snapshotLock{};

    std::vector<NodeComponent<Camera>> m_cameraComponents;

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
