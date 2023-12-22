#pragma once

#include <map>
#include <unordered_map>
#include <vector>
#include <tuple>

#include <fmt/format.h>

#include "ki/size.h"
#include "ki/uuid.h"

#include "backend/DrawOptions.h"

#include "model/Node.h"

#include "registry/MeshType.h"


class Registry;

//
// NOTE KI program key is REQUIRED for sorting "gull back face" draws
// next to each other to avoid redundant state changes
// => relies into fact that std::map is sorted by this
//
 struct ProgramKey {
    ProgramKey(
        ki::program_id programID,
        int typePriority,
        const backend::DrawOptions& drawOptions) noexcept
        : programID(programID),
        typePriority(typePriority),
        renderBack(drawOptions.renderBack),
        wireframe(drawOptions.wireframe)
    {};

    const std::string str() const noexcept
    {
        return fmt::format(
            "<PROGRAM_KEY: id={}, pri={}, renderBack={}, wireframe={}>",
            programID, typePriority, renderBack, wireframe);
    }

    bool operator<(const ProgramKey & o) const noexcept {
        // NOTE KI renderBack & wireframe goes into separate render always due to GL state
        // => reduce state changes via sorting
        return std::tie(typePriority, programID, renderBack, wireframe) <
            std::tie(o.typePriority, o.programID, o.renderBack, o.wireframe);
    }

    const int typePriority;
    const ki::program_id programID;
    const bool renderBack;
    const bool wireframe;
};

// https://stackoverflow.com/questions/5733254/how-can-i-create-my-own-comparator-for-a-map
struct MeshTypeKey {
    MeshTypeKey(const MeshType* type);

    bool operator<(const MeshTypeKey& o) const;

    const MeshType* type;
};

using NodeVector = std::vector<Node*>;
using MeshTypeMap = std::map<MeshTypeKey, NodeVector>;
using ProgramTypeMap = std::map<ProgramKey, MeshTypeMap>;


class NodeRegistry final
{
public:
    NodeRegistry(
        const Assets& assets,
        std::shared_ptr<std::atomic<bool>> alive);

    ~NodeRegistry();

    void prepare(
        Registry* registry);

    void attachListeners();

    //inline bool containsNode(const int id) const noexcept
    //{
    //    const auto& it = m_idToNode.find(id);
    //    return it != m_idToNode.end();
    //}

    // @return node null if not found
    inline Node* getNode(const ki::node_id id) const noexcept
    {
        const auto& it = m_idToNode.find(id);
        return it != m_idToNode.end() ? it->second : nullptr;
    }

    // @return node null if not found
    inline Node* getNode(const uuids::uuid& id) const noexcept
    {
        const auto& it = m_uuidToNode.find(id);
        return it != m_uuidToNode.end() ? it->second : nullptr;
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
        Node* node,
        const uuids::uuid& parentId) noexcept;

    inline const NodeVector* getChildren(const Node& parent) const noexcept {
        const auto& it = m_parentToChildren.find(parent.m_id);
        return it != m_parentToChildren.end() ? &it->second : nullptr;
    }

    inline Node* getActiveNode() const noexcept { return m_activeNode; }
    inline Node* getActiveCamera2() const noexcept { return m_activeCamera; }

    Node* getNextCamera(Node* srcNode, int offset) const noexcept;

    Node* findDefaultCamera() const;

    inline const Material& getSelectionMaterial() const noexcept {
        return m_selectionMaterial;
    }

    void setSelectionMaterial(const Material& material) {
        m_selectionMaterial = material;
    }

private:
    void setActiveNode(Node* node);
    void setActiveCamera(Node* node);

    void attachNode(
        Node* node,
        const uuids::uuid& uuid,
        const uuids::uuid& parentUUID,
        ki::node_id parentId) noexcept;

    void insertNode(NodeVector& list, Node* node);

    void bindPendingChildren();

    void bindNode(
        const uuids::uuid& uuid,
        Node* node);

    bool bindParent(
        Node* node,
        const uuids::uuid& uuid,
        const uuids::uuid& parentUUID,
        ki::node_id parentId);

    void bindChildren(
        const uuids::uuid& parentUUID);

    void bindSkybox(
        Node* node) noexcept;

public:
    ProgramTypeMap allNodes;
    ProgramTypeMap solidNodes;
    ProgramTypeMap alphaNodes;
    ProgramTypeMap spriteNodes;
    ProgramTypeMap blendedNodes;
    ProgramTypeMap invisibleNodes;
    ProgramTypeMap physicsNodes;

    Node* m_root{ nullptr };
    Node* m_dirLight{ nullptr };

    Node* m_skybox{ nullptr };

    NodeVector m_cameras;

    NodeVector m_pointLights;
    NodeVector m_spotLights;

private:
    const Assets& m_assets;

    std::shared_ptr<std::atomic<bool>> m_alive;

    Registry* m_registry{ nullptr };

    std::unordered_map<ki::node_id, Node*> m_idToNode;
    std::unordered_map<uuids::uuid, Node*> m_uuidToNode;

    std::unordered_map<uuids::uuid, std::vector<std::tuple<const uuids::uuid, Node*>>> m_pendingChildren;

    std::unordered_map<ki::node_id, NodeVector> m_parentToChildren;

    Node* m_activeNode{ nullptr };
    Node* m_activeCamera{ nullptr };

    Material m_selectionMaterial;

    mutable int m_taggedCount{ -1 };
    mutable int m_selectedCount{ -1 };
};
