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

#include "component/NodeComponent.h"

#include "model/NodeState.h"
#include "model/Snapshot.h"

struct Material;

struct CreateState;
struct UpdateContext;

class Registry;

struct EntitySSBO;

namespace physics {
    class PhysicsSystem;
}

namespace mesh
{
    class MeshYype;
}

class NodeRegistry final
{
    friend class Node;

public:
    static void init() noexcept;
    static void release() noexcept;
    static NodeRegistry& get() noexcept;

    NodeRegistry();
    NodeRegistry& operator=(const NodeRegistry&) = delete;

    ~NodeRegistry();

    void clear();

    void prepare(
        Registry* registry);

    void updateWT(const UpdateContext& ctx);
    void postUpdateWT(const UpdateContext& ctx);

    // @return first invalid index, -1 for no errors
    int validateModelMatrices();

    void updateModelMatrices();
    void updateModelMatrices(const Node* node);

    void prepareUpdateRT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);
    // @return [min-dirty, max-dirty]
    std::pair<int, int> updateEntity(const UpdateContext& ctx);

    void snapshotPending();
    void snapshotRT();

    void snapshot(
        std::vector<Snapshot>& src,
        std::vector<Snapshot>& dst);

    std::vector<Node*>& getCachedNodesWT();

    const std::vector<Node*> getCachedNodesRT() const noexcept
    {
        return m_cachedNodesRT;
    }

    void attachListeners();

    void handleNodeAdded(Node* node);
    void handleNodeRemoved(Node* node);

    void notifyPendingChanges();

    // @return root if root is prepared for RT
    Node* getRootRT() const noexcept {
        return m_rootRT.toNode();
    }

    // @return root if root is prepared for WT
    Node* getRootWT() const noexcept {
        return m_rootWT.toNode();
    }

    void attachNode(
        const pool::NodeHandle nodeHandle,
        ki::node_id parentId,
        ki::socket_id socketId,
        const CreateState& state) noexcept;

    void detachNode(
        const pool::NodeHandle nodeHandle) noexcept;

    void disposeNode(
        const pool::NodeHandle nodeHandle) noexcept;

    void changeParent(
        const pool::NodeHandle nodeHandle,
        ki::node_id parentId,
        ki::socket_id socketId) noexcept;

    void viewportChanged(
        uint8_t layer,
        glm::uvec2 aspectRatio) noexcept;

    inline Node* getActiveNode() const noexcept { return m_activeNode.toNode(); }

    uint32_t getNodeCount() const noexcept
    {
        return static_cast<uint32_t>(m_handles.size());
    }

    //ki::level_id getLevel() const noexcept {
    //    std::lock_guard lock(m_lock);
    //    return m_nodeLevel;
    //}

    //void withLock(const std::function<void(NodeRegistry&)>& fn);

    uint32_t getParentIndex(uint32_t entityIndex) const noexcept
    {
        return m_parentIndeces[entityIndex];
    }

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

    //const Snapshot* getSnapshotWT(uint32_t entityIndex) const noexcept
    //{
    //    return m_snapshotsWT.size() > entityIndex ? &m_snapshotsWT[entityIndex] : nullptr;
    //}

    //bool hasSnapshotWT(uint32_t entityIndex) const noexcept
    //{
    //    return m_snapshotsWT.size() > entityIndex;
    //}

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

    std::vector<bool>& getDirtyEntities() noexcept
    {
        return m_dirtyEntities;
    }

    pool::NodeHandle findTagged(
        ki::tag_id tagId);

    std::vector<pool::NodeHandle> findTaggedAll(
        ki::tag_id tagId);

    pool::NodeHandle findTaggedChild(
        pool::NodeHandle handle,
        ki::tag_id tagId);

private:
    void setActiveNode(pool::NodeHandle node);

    void bindNode(
        const pool::NodeHandle nodeHandle,
        const CreateState& state);

    void unbindNode(
        const pool::NodeHandle nodeHandle);

    bool bindParent(
        const pool::NodeHandle nodeHandle,
        ki::node_id parentId);

    bool unbindParent(
        const pool::NodeHandle nodeHandle);

    bool bindParentSocket(
        const pool::NodeHandle nodeHandle,
        ki::socket_id socketId);

    void bindSkybox(
        pool::NodeHandle handle) noexcept;

    void cacheNodes(
        std::vector<Node*>& cache,
        ki::level_id& cacheLevel);

    void updateBounds(
        const UpdateContext& ctx,
        NodeState& state,
        const NodeState& parentState,
        const Node* node,
        const physics::PhysicsSystem& physicsSystem);

    void logDebugInfo(const std::string& err, uint32_t entityIndex) const;

public:
    pool::NodeHandle m_skybox{};

private:
    ki::node_id m_rootId{};
    pool::NodeHandle m_rootHandle{};

    pool::NodeHandle m_rootWT{};
    pool::NodeHandle m_rootRT{};
    uint32_t m_rootIndex{ 0 };
    bool m_rootPreparedRT{ false };

    // Free deleted slots
    // => reuse slots after RT is synced with WT
    std::vector<uint32_t> m_freeIndeces;

    // Delete node after RT is synced with WT
    std::vector<pool::NodeHandle> m_pendingRemoved;

    std::vector<pool::NodeHandle> m_pendingAdded;

    // Entries matched by index
    // INDEX = entityIndex
    std::vector<pool::NodeHandle> m_handles;
    // INDEX = entityIndex
    std::vector<uint32_t> m_parentIndeces;
    // INDEX = entityIndex
    std::vector<NodeState> m_states;
    // INDEX = entityIndex
    std::vector<Snapshot> m_snapshotsPending;
    // INDEX = entityIndex
    std::vector<Snapshot> m_snapshotsRT;
    // INDEX = entityIndex
    std::vector<EntitySSBO> m_entities;
    // INDEX = entityIndex
    std::vector<bool> m_dirtyEntities;

    // INDEX = entityIndex
    std::vector<Node*> m_cachedNodesWT;
    // INDEX = entityIndex
    std::vector<Node*> m_cachedNodesRT;

    ki::level_id m_nodeLevel{ 0 };
    ki::level_id m_cachedNodeLevelWT{ 0 };
    ki::level_id m_cachedNodeLevelRT{ 0 };

    Registry* m_registry{ nullptr };

    mutable std::mutex m_snapshotLock;

    pool::NodeHandle m_activeNode{};

    std::vector<std::pair<glm::uvec2, bool>> m_layerInfos;
};
