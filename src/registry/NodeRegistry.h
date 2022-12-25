#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <tuple>

#include <ki/uuid.h>

#include "backend/DrawOptions.h"

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
    ShaderKey(int shaderID, const backend::DrawOptions& drawOptions) noexcept
        : shaderID(shaderID),
        drawOptions(drawOptions) {};

    const int shaderID;
    const backend::DrawOptions drawOptions;

    bool operator<(const ShaderKey & o) const noexcept {
        const bool drawOrder = drawOptions < o.drawOptions;
        return std::tie(shaderID, drawOrder) <
            std::tie(o.shaderID, drawOrder);
    }
};

class Batch;
class MaterialRegistry;
class ModelRegistry;

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

    void prepare(
        Batch* batch,
        MaterialRegistry * materialRegistry,
        ModelRegistry* modelRegistry);

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

    void attachNodes();

    int countSelected() const noexcept;

    void changeParent(Node* node, uuids::uuid parentId) noexcept;

    Node* getParent(const Node& node) const noexcept;
    const NodeVector* getChildren(const Node& node) const noexcept;

private:
    void bindPendingChildren();

    void bindNode(
        Node* node);

    bool bindParent(
        Node* child);

    void bindChildren(
        Node* parent);

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

    Batch* m_batch{ nullptr };
    MaterialRegistry* m_materialRegistry{ nullptr };
    ModelRegistry* m_modelRegistry{ nullptr };

    std::mutex m_load_lock;
    std::condition_variable m_waitCondition;

    std::map<uuids::uuid, NodeVector> m_pendingChildren;

    std::map<int, Node*> m_childToParent;
    std::map<int, NodeVector> m_parentToChildren;

    NodeVector m_pendingNodes;

    std::vector<NodeListener> m_listeners;

};
