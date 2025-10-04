#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>
#include <mutex>
#include <functional>

#include <fmt/format.h>

#include "ki/size.h"

#include "event/Listen.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "component/NodeComponent.h"

#include "model/NodeState.h"
#include "model/Snapshot.h"

struct Material;

namespace model {
    struct CreateState;
}

struct UpdateContext;

class Registry;
class Engine;

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
    friend class model::Node;

public:
    static void init() noexcept;
    static void release() noexcept;
    static NodeRegistry& get() noexcept;

    NodeRegistry();
    NodeRegistry& operator=(const NodeRegistry&) = delete;

    ~NodeRegistry();

    void clear();

    void prepare(
        Engine* engine);

    void updateWT(const UpdateContext& ctx);
    void postUpdateWT(const UpdateContext& ctx);

    // @return first invalid index, -1 for no errors
    int validateModelMatrices();

    void updateModelMatrices();
    void updateModelMatrices(const model::Node* node);

    void prepareUpdateRT(const UpdateContext& ctx);
    void updateRT(const UpdateContext& ctx);
    // @return [min-dirty, max-dirty]
    std::pair<int, int> updateEntity(const UpdateContext& ctx);

    void snapshotPending();
    void snapshotRT();

    void snapshot(
        std::vector<model::Snapshot>& src,
        std::vector<model::Snapshot>& dst);

    std::vector<model::Node*>& getCachedNodesWT();

    const std::vector<model::Node*> getCachedNodesRT() const noexcept
    {
        return m_cachedNodesRT;
    }

    void attachListeners();

    void handleNodeAdded(model::Node* node);
    void handleNodeRemoved(model::Node* node);

    void notifyPendingChanges();

    // @return root if root is prepared for RT
    model::Node* getRootRT() const noexcept {
        return m_rootRT.toNode();
    }

    // @return root if root is prepared for WT
    model::Node* getRootWT() const noexcept {
        return m_rootWT.toNode();
    }

    void attachNode(
        const pool::NodeHandle nodeHandle,
        ki::node_id parentId,
        ki::socket_id socketId,
        const model::CreateState& state) noexcept;

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

    inline model::Node* getActiveNode() const noexcept { return m_activeNode.toNode(); }

    uint32_t getNodeCount() const noexcept
    {
        return static_cast<uint32_t>(m_sortedNodes.size());
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

    const model::Node* getParent(uint32_t entityIndex) const noexcept
    {
        return m_handles[m_parentIndeces[entityIndex]].toNode();
    }

    const model::NodeState& getParentState(uint32_t entityIndex) const noexcept
    {
        return m_states[m_parentIndeces[entityIndex]];
    }

    model::NodeState& modifyState(uint32_t entityIndex) noexcept
    {
        return m_states[entityIndex];
    }

    const model::NodeState& getState(uint32_t entityIndex) const noexcept
    {
        return m_states[entityIndex];
    }

    //const model::Snapshot* getSnapshotWT(uint32_t entityIndex) const noexcept
    //{
    //    return m_snapshotsWT.size() > entityIndex ? &m_snapshotsWT[entityIndex] : nullptr;
    //}

    //bool hasSnapshotWT(uint32_t entityIndex) const noexcept
    //{
    //    return m_snapshotsWT.size() > entityIndex;
    //}

    const model::Snapshot* getSnapshotRT(uint32_t entityIndex) const noexcept
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

    bool isPendingRemoveConflict(uint32_t entityIndex);

    void sortNodes(
        const pool::NodeHandle nodeHandle,
        bool add,
        bool remove);

    void bindNode(
        const pool::NodeHandle nodeHandle,
        const model::CreateState& state);

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
        std::vector<model::Node*>& cache,
        ki::level_id& cacheLevel);

    void updateBounds(
        const UpdateContext& ctx,
        model::NodeState& state,
        const model::NodeState& parentState,
        const model::Node* node,
        const physics::PhysicsSystem& physicsSystem);

    void logDebugInfo(const std::string& err, uint32_t entityIndex) const;

public:
    pool::NodeHandle m_skybox{};

private:
    event::Listen m_listen_node_add;
    event::Listen m_listen_node_remove;
    event::Listen m_listen_node_dispose;
    event::Listen m_listen_node_activate;
    event::Listen m_listen_node_added;
    event::Listen m_listen_type_prepare_view;
    event::Listen m_listen_viewport_changed;

    ki::node_id m_rootId{};
    pool::NodeHandle m_rootHandle{};

    pool::NodeHandle m_rootWT{};
    pool::NodeHandle m_rootRT{};
    uint32_t m_rootEntityIndex{ 0 };
    bool m_rootPreparedRT{ false };

    // Free deleted slots (entityIndeces)
    // => reuse slots after RT is synced with WT
    std::vector<uint32_t> m_freeIndeces;

    // Delete node after RT is synced with WT
    std::vector<pool::NodeHandle> m_pendingRemoved;

    std::vector<pool::NodeHandle> m_pendingAdded;

    // topologically sorted nodes (entityIndeces)
    // INDEX = nodeIndex
    std::vector<uint32_t> m_sortedNodes;

    // Entries matched by index
    // INDEX = entityIndex
    std::vector<pool::NodeHandle> m_handles;
    // INDEX = entityIndex
    std::vector<uint32_t> m_parentIndeces;
    // INDEX = entityIndex
    std::vector<model::NodeState> m_states;
    // INDEX = entityIndex
    std::vector<model::Snapshot> m_snapshotsPending;
    // INDEX = entityIndex
    std::vector<model::Snapshot> m_snapshotsRT;
    // INDEX = entityIndex
    std::vector<EntitySSBO> m_entities;
    // INDEX = entityIndex
    std::vector<bool> m_dirtyEntities;

    // INDEX = entityIndex
    std::vector<model::Node*> m_cachedNodesWT;
    // INDEX = entityIndex
    std::vector<model::Node*> m_cachedNodesRT;

    ki::level_id m_nodeLevel{ 0 };
    ki::level_id m_cachedNodeLevelWT{ 0 };
    ki::level_id m_cachedNodeLevelRT{ 0 };

    Engine* m_engine{ nullptr };

    mutable std::mutex m_snapshotLock;

    pool::NodeHandle m_activeNode{};

    std::vector<std::pair<glm::uvec2, bool>> m_layerInfos;
};
