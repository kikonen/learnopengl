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

#include "registry/MeshType.h"

//
// NOTE KI shader key is REQUIRED for sorting "gull back face" draws
// next to each other to avoid redundant state changes
// => relies into fact that std::map is sorted by this
//
 struct ShaderKey {
    ShaderKey(int shaderID, bool renderBack) noexcept
        : shaderID(shaderID),
        renderBack(renderBack) {};

    int shaderID;
    bool renderBack;
    
    bool operator<(const ShaderKey & o)  const noexcept {
        return std::tie(shaderID, renderBack) < std::tie(o.shaderID, o.renderBack);
    }
};

class MaterialRegistry;
class MeshRegistry;

enum class NodeOperation {
    ADDED
};

using GroupVector = std::vector<Group*>;

using NodeVector = std::vector<Node*>;
using MeshTypeMap = std::map<MeshType*, NodeVector>;
using ShaderTypeMap = std::map<ShaderKey, MeshTypeMap>;

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
        MeshType* type,
        Node* node) noexcept;

    // @return node null if not found
    Node* getNode(const int objectID) const noexcept;

    // @return node null if not found
    Node* getNode(const uuids::uuid& id) const noexcept;

    void selectNodeByObjectId(int objectID, bool append) const noexcept;

    void addViewPort(std::shared_ptr<Viewport> viewport) noexcept;

    void attachNodes(
        MaterialRegistry& materialRegistry,
        MeshRegistry& meshRegistry);

    int countSelected() const noexcept;

    void changeParent(Node* node, uuids::uuid parentId) noexcept;

    Node* getParent(const Node& node) const noexcept;
    const NodeVector* getChildren(const Node& node) const noexcept;

private:
    void bindPendingChildren(
        MaterialRegistry& materialRegistry,
        MeshRegistry& meshRegistry);

    void bindNode(
        Node* node,
        MaterialRegistry& materialRegistry,
        MeshRegistry& meshRegistry);

    bool bindParent(
        Node* child,
        MaterialRegistry& materialRegistry,
        MeshRegistry& meshRegistry);

    void bindChildren(
        Node* parent,
        MaterialRegistry& materialRegistry,
        MeshRegistry& meshRegistry);

    void notifyListeners(Node* node, NodeOperation operation);

public:
    std::map<int, Node*> objectIdToNode;
    std::map<uuids::uuid, Node*> idToNode;

    ShaderTypeMap allNodes;
    ShaderTypeMap solidNodes;
    ShaderTypeMap alphaNodes;
    ShaderTypeMap blendedNodes;
    ShaderTypeMap invisibleNodes;

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
