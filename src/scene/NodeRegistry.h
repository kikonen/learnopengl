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
    ShaderKey(int shaderID, bool renderBack) : shaderID(shaderID), renderBack(renderBack) {};
    int shaderID;
    bool renderBack;

    bool operator<(const ShaderKey& o)  const {
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

    void addGroup(Group* group);

    void addNode(Node* node);

    // @return node null if not found
    Node* const getNode(const int objectID);

    // @return node null if not found
    Node* const getNode(const uuids::uuid& id);

    void const selectNodeByObjectId(int objectID, bool append);

    void addViewPort(std::shared_ptr<Viewport> viewport);

    void attachNodes();

    int const countSelected() const;

    Node* const getParent(const Node& node);
    NodeVector* const getChildren(const Node& node);

private:
    void bindNode(Node* node);
    bool bindParent(Node* child);
    void bindChildren(Node* parent);

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

    Node* cameraNode = nullptr;
    Node* dirLight = nullptr;

    NodeVector pointLights;
    NodeVector spotLights;

    GroupVector groups;

private:
    std::mutex load_lock;
    std::condition_variable waitCondition;

    std::map<uuids::uuid, NodeVector> pendingChildren;

    std::map<int, Node*> childToParent;
    std::map<int, NodeVector> parentToChildren;

    NodeVector pendingNodes;

};
