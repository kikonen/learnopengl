#include "NodeBuilder.h"

#include <string>
#include <vector>
#include <fstream>
#include <span>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"

#include <engine/AsyncLoader.h>

#include "Context.h"
#include "Loaders.h"
#include "NodeData.h"
#include "ResolvedNode.h"

#include "SceneLoader.h"

#include "loader_util.h"

namespace
{
    const ki::node_id* findAlias(
        loader::BaseId aliasId,
        std::vector<std::pair<std::string, ki::node_id>>& aliases)
    {
        const auto& it = std::find_if(
            aliases.cbegin(),
            aliases.cend(),
            [&aliasId](const auto& e) { return e.first == aliasId.m_path; });
        return it != aliases.end() ? &(*it).second : nullptr;
    }
}

namespace loader
{
    NodeBuilder::NodeBuilder(
        SceneLoader* sceneLoader,
        std::shared_ptr<Context> ctx,
        std::shared_ptr<Loaders> loaders)
        : m_sceneLoader{ sceneLoader },
        m_ctx{ ctx },
        m_loaders{ loaders }
    { }

    NodeBuilder::~NodeBuilder() = default;

    void NodeBuilder::loadedNode(
        const NodeData& nodeData,
        bool success)
    {
        m_sceneLoader->loadedNode(nodeData, success);
    }

    void NodeBuilder::addResolvedNode(
        const ResolvedNode& resolved)
    {
        std::lock_guard lock(m_resolvedNodesLock);
        m_resolvedNodes.push_back(resolved);
    }

    // TODO KI need to change node add logic to happen via commands
    // => i.e. api which can be used also from Lua
    bool NodeBuilder::resolveNode(
        const ki::node_id ownerId,
        const NodeData& baseData)
    {
        if (!baseData.enabled) {
            return false;
        }

        m_ctx->m_asyncLoader->addLoader(m_ctx->m_alive, [this, ownerId, &baseData]() {
            std::vector<std::pair<std::string, ki::node_id>> aliases;
            resolveNode(ownerId, baseData, aliases, true);
            loadedNode(baseData, true);
        });

        return true;
    }

    void NodeBuilder::resolveNode(
        const ki::node_id ownerId,
        const NodeData& baseData,
        std::vector<std::pair<std::string, ki::node_id>>& aliases,
        bool root)
    {
        try {
            if (!baseData.clones) {
                resolveNodeClone(ownerId, baseData, aliases, false, 0);
            }
            else {
                int cloneIndex = 0;
                for (const auto& cloneData : *baseData.clones) {
                    if (!*m_ctx->m_alive) return;
                    resolveNodeClone(ownerId, cloneData, aliases, true, cloneIndex);
                    cloneIndex++;
                }
            }
        }
        catch (const std::runtime_error& ex) {
            KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", ex.what()));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", ex));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", ex));
        }
        catch (...) {
            KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", "UNKNOWN_ERROR"));
        }
    }

    void NodeBuilder::resolveNodeClone(
        const ki::node_id ownerId,
        const NodeData& cloneData,
        std::vector<std::pair<std::string, ki::node_id>>& aliases,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx->m_alive) return;

        if (!cloneData.enabled) {
            return;
        }

        const auto& repeat = cloneData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    if (!*m_ctx->m_alive) return;

                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{
                        x * repeat.xStep,
                        y * repeat.yStep,
                        z * repeat.zStep };

                    resolveNodeCloneRepeat(
                        ownerId,
                        cloneData,
                        aliases,
                        cloned,
                        cloneIndex,
                        tile,
                        tilePositionOffset);
                }
            }
        }
    }

    void NodeBuilder::resolveNodeCloneRepeat(
        const ki::node_id ownerId,
        const NodeData& cloneData,
        std::vector<std::pair<std::string, ki::node_id>>& aliases,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset)
    {
        if (!cloneData.enabled) return;
        if (!*m_ctx->m_alive) return;

        auto [handle, state] = createNode(
            cloneData,
            aliases,
            cloneIndex,
            tile,
            cloneData.clonePositionOffset + tilePositionOffset);

        ki::node_id parentId;
        if (cloneData.parentBaseId.empty()) {
            parentId = ownerId;
        }
        else {
            auto [id, _] = resolveNodeId(
                cloneData.typeId,
                cloneData.parentBaseId,
                cloneIndex,
                tile);
            parentId = id;
        }

        if (!cloneData.aliasBaseId.empty())
        {
            std::string key = expandMacros(cloneData.aliasBaseId.m_path, cloneIndex, tile);

            aliases.push_back({ key, handle.toId()});
        }

        ResolvedNode resolved{
            parentId,
            handle,
            cloneData,
            state,
        };

        addResolvedNode(resolved);

        if (cloneData.children) {
            for (const auto& childData : *cloneData.children) {
                resolveNode(
                    handle.toId(),
                    childData,
                    aliases,
                    false);
            }
        }
    }

    std::pair<pool::NodeHandle, CreateState> NodeBuilder::createNode(
        const NodeData& nodeData,
        std::vector<std::pair<std::string, ki::node_id>>& aliases,
        const int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& positionOffset)
    {
        auto& l = *m_loaders;

        const auto* type = findNodeType(nodeData.typeId);
        if (!type) {
            throw fmt::format("type missing: node={}, type={}", nodeData.str(), nodeData.typeId.str());
        }

        ki::node_id nodeId{ 0 };
        std::string resolvedSID;
        {
            auto [k, v] = resolveNodeId(
                nodeData.typeId,
                nodeData.baseId,
                cloneIndex,
                tile);
            nodeId = k;
            resolvedSID = v;
        }

        KI_INFO_OUT("NODE_SID: " + resolvedSID);

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);

        node->setName(resolvedSID);

        node->m_typeHandle = type->toHandle();

        if (!nodeData.ignoredByBaseId.empty())
        {
            auto* ignoredBy = findAlias(nodeData.ignoredByBaseId, aliases);
            if (!ignoredBy) {
                throw fmt::format("ignored_by missing: node={}, ignored_by={}", nodeData.str(), nodeData.ignoredByBaseId.str());
            }
            node->m_ignoredBy = *ignoredBy;
        }

        const glm::vec3 pos = nodeData.position + positionOffset;

        CreateState state;
        state.m_position = pos;
        state.m_scale = nodeData.scale;
        state.m_rotation = util::degreesToQuat(nodeData.rotation);
        state.m_tilingX = nodeData.tilingX;
        state.m_tilingY = nodeData.tilingY;

        return { handle, state };
    }
}
