#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <mutex>
#include <functional>

#include <fmt/format.h>

#include "ki/size.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "component/CameraComponent.h"
#include "component/NodeComponent.h"

#include "model/NodeState.h"
#include "model/Snapshot.h"

struct Material;

struct UpdateContext;

class Registry;
class NodeSnapshotRegistry;

struct EntitySSBO;

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

    void prepareUpdateRT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);
    void updateEntity(const UpdateContext& ctx);

    void snapshotWT();
    void snapshotPending();
    void snapshotRT();

    void snapshot(
        std::vector<Snapshot>& src,
        std::vector<Snapshot>& dst);

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
        return static_cast<uint32_t>(m_handles.size());
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

    pool::NodeHandle getParentHandle(uint32_t entityIndex) const noexcept
    {
        return m_handles[m_parentIndeces[entityIndex]];
    }

    const Node* getParent(uint32_t entityIndex) const noexcept
    {
        return m_handles[m_parentIndeces[entityIndex]].toNode();
    }

    const NodeState& getParentState(uint32_t entityIndex) const noexcept
    {
        return m_states[m_parentIndeces[entityIndex]];
    }

    NodeState& modifyState(uint32_t entityIndex) noexcept
    {
        return m_states[entityIndex];
    }

    const NodeState& getState(uint32_t entityIndex) const noexcept
    {
        return m_states[entityIndex];
    }

    const Snapshot* getSnapshotWT(uint32_t entityIndex) const noexcept
    {
        return m_snapshotsWT.size() > entityIndex ? &m_snapshotsWT[entityIndex] : nullptr;
    }

    bool hasSnapshotWT(uint32_t entityIndex) const noexcept
    {
        return m_snapshotsWT.size() > entityIndex;
    }

    const Snapshot* getSnapshotRT(uint32_t entityIndex) const noexcept
    {
        if (m_snapshotsRT.size() <= entityIndex) {
            logDebugInfo("MISSING_SNAPSHOT", entityIndex);
        }
        return m_snapshotsRT.size() > entityIndex ? &m_snapshotsRT[entityIndex] : nullptr;
    }

    bool hasSnapshotRT(uint32_t entityIndex) const noexcept
    {
        return m_snapshotsRT.size() > entityIndex;
    }

    const std::vector<EntitySSBO>& getEntities() const noexcept
    {
        return m_entities;
    }

    void cacheNodesWT();
    void cacheNodesRT();

    void logDebugInfo(const std::string& err, uint32_t entityIndex) const;

private:
    void setActiveNode(pool::NodeHandle node);
    void setActiveCameraNode(pool::NodeHandle node);

    void attachNode(
        const ki::node_id nodeId,
        const ki::node_id parentId,
        const NodeState& state) noexcept;

    void bindNode(
        const ki::node_id nodeId,
        const NodeState& state);

    bool bindParent(
        const ki::node_id nodeId,
        const ki::node_id parentId);

    void bindSkybox(
        pool::NodeHandle handle) noexcept;

    void cacheNodes(std::vector<Node*>& cache);

public:
    pool::NodeHandle m_skybox{};

private:
    ki::node_id m_rootId{ 0 };

    pool::NodeHandle m_rootWT{};
    pool::NodeHandle m_rootRT{};
    uint32_t m_rootIndex{ 0 };
    bool m_rootPreparedRT{ false };

    // Entries matched by index
    std::vector<pool::NodeHandle> m_handles;
    std::vector<uint32_t> m_parentIndeces;
    std::vector<NodeState> m_states;
    std::vector<Snapshot> m_snapshotsWT;
    std::vector<Snapshot> m_snapshotsPending;
    std::vector<Snapshot> m_snapshotsRT;
    std::vector<EntitySSBO> m_entities;

    std::vector<Node*> m_cachedNodesWT;
    std::vector<Node*> m_cachedNodesRT;

    ki::level_id m_nodeLevel{ 0 };
    ki::level_id m_cachedNodeLevel{ 0 };

    Registry* m_registry{ nullptr };

    mutable std::mutex m_snapshotLock;

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
