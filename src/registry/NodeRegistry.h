#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <tuple>

#include <ki/uuid.h>

#include "asset/QuadVBO.h"

#include "model/Group.h"
#include "model/Node.h"
#include "model/Viewport.h"

#include "NodeType.h"


class MaterialRegistry;

struct ShaderKey {
    ShaderKey(int shaderID, bool renderBack) noexcept
        : shaderID(shaderID),
        renderBack(renderBack) {};
    int shaderID;
    bool renderBack;

    bool operator<(const ShaderKey& o)  const noexcept {
        return std::tie(shaderID, renderBack) < std::tie(o.shaderID, o.renderBack);
    }
};

enum class NodeOperation {
    ADDED
};

using GroupVector = std::vector<Group*>;

using NodeVector = std::vector<Node*>;
using NodeTypeMap = std::map<NodeType*, NodeVector>;
using ShaderTypeMap = std::map<const ShaderKey, NodeTypeMap>;

using ViewportVector = std::vector<std::shared_ptr<Viewport>>;

using NodeListener = std::function<void(Node*, NodeOperation)>;

class NodeRegistry final
{
public:
    NodeRegistry(const Assets& assets);

    ~NodeRegistry();

    void prepare();

    void addListener(NodeListener& listener);

    void addGroup(Group* group) noexcept;

    void addNode(
        NodeType* type,
        Node* node) noexcept;

    // @return node null if not found
    Node* getNode(const int objectID) const noexcept;

    // @return node null if not found
    Node* getNode(const uuids::uuid& id) const noexcept;

    void selectNodeByObjectId(int objectID, bool append) const noexcept;

    void addViewPort(std::shared_ptr<Viewport> viewport) noexcept;

    void attachNodes(
        MaterialRegistry& materialRegistry);

    int countSelected() const noexcept;

    void changeParent(Node* node, uuids::uuid parentId) noexcept;

    Node* getParent(const Node& node) const noexcept;
    const NodeVector* getChildren(const Node& node) const noexcept;

private:
    void bindPendingChildren(
        MaterialRegistry& materialRegistry);

    void bindNode(
        Node* node,
        MaterialRegistry& materialRegistry);

    bool bindParent(
        Node* child,
        MaterialRegistry& materialRegistry);

    void bindChildren(
        Node* parent,
        MaterialRegistry& materialRegistry);

    void notifyListeners(Node* node, NodeOperation operation);

public:
    std::map<int, Node*> objectIdToNode;
    std::map<uuids::uuid, Node*> idToNode;

    ShaderTypeMap allNodes;
    ShaderTypeMap solidNodes;
    ShaderTypeMap alphaNodes;
    ShaderTypeMap blendedNodes;

    ViewportVector viewports;

    Node* m_root = nullptr;
    NodeVector m_cameraNodes;
    Node* m_dirLight = nullptr;

    NodeVector m_pointLights;
    NodeVector m_spotLights;

    GroupVector groups;

private:
    const Assets& assets;

    std::mutex m_load_lock;
    std::condition_variable m_waitCondition;

    std::map<uuids::uuid, NodeVector> m_pendingChildren;

    std::map<int, Node*> m_childToParent;
    std::map<int, NodeVector> m_parentToChildren;

    NodeVector m_pendingNodes;

    std::vector<NodeListener> m_listeners;

};
