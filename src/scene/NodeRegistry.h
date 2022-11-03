#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <tuple>

#include <ki/uuid.h>

#include "NodeType.h"

#include "model/Group.h"
#include "model/Node.h"
#include "model/Viewport.h"

class Sccene;

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

using GroupVector = std::vector<Group*>;

using NodeVector = std::vector<Node*>;
using NodeTypeMap = std::map<NodeType*, NodeVector>;
using ShaderTypeMap = std::map<const ShaderKey, NodeTypeMap>;

using ViewportVector = std::vector<std::shared_ptr<Viewport>>;

class NodeRegistry final
{
public:
    NodeRegistry(Scene& scene);

    ~NodeRegistry();

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

    void attachNodes() noexcept;

    int countSelected() const noexcept;

    void changeParent(Node* node, uuids::uuid parentId) noexcept;

    Node* getParent(const Node& node) const noexcept;
    const NodeVector* getChildren(const Node& node) const noexcept;

private:
    void bindPendingChildren() noexcept;
    void bindNode(Node* node) noexcept;
    bool bindParent(Node* child) noexcept;
    void bindChildren(Node* parent) noexcept;

public:
    const Assets& assets;
    Scene& scene;

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
    std::mutex m_load_lock;
    std::condition_variable m_waitCondition;

    std::map<uuids::uuid, NodeVector> m_pendingChildren;

    std::map<int, Node*> m_childToParent;
    std::map<int, NodeVector> m_parentToChildren;

    NodeVector m_pendingNodes;

};
