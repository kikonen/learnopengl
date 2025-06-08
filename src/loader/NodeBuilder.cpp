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
#include "NodeRoot.h"
#include "ResolvedNode.h"

#include "SceneLoader.h"
#include "NodeTypeBuilder.h"

#include "loader_util.h"

namespace loader
{
    NodeBuilder::NodeBuilder(
        SceneLoader* sceneLoader,
        std::shared_ptr<Context> ctx,
        std::shared_ptr<Loaders> loaders)
        : m_sceneLoader{ sceneLoader },
        m_ctx{ ctx },
        m_loaders{ loaders },
        m_nodeTypeBuilder{ std::make_unique<NodeTypeBuilder>(m_loaders) }
    { }

    NodeBuilder::~NodeBuilder() = default;

    void NodeBuilder::loadedNode(
        const NodeRoot& nodeRoot,
        bool success)
    {
        m_sceneLoader->loadedNode(nodeRoot, success);
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
        const ki::node_id rootId,
        const NodeRoot& nodeRoot)
    {
        if (!nodeRoot.base.enabled) {
            return false;
        }

        m_ctx->m_asyncLoader->addLoader(m_ctx->m_alive, [this, rootId, &nodeRoot]() {
            try {
                if (nodeRoot.clones.empty()) {
                    pool::TypeHandle typeHandle{};
                    resolveNodeClone(typeHandle, rootId, nodeRoot, nodeRoot.base, false, 0);
                }
                else {
                    pool::TypeHandle typeHandle{};

                    int cloneIndex = 0;
                    for (auto& cloneData : nodeRoot.clones) {
                        if (!*m_ctx->m_alive) return;
                        typeHandle = resolveNodeClone(typeHandle, rootId, nodeRoot, cloneData, true, cloneIndex);
                        if (!nodeRoot.base.shareType) {
                            typeHandle = pool::TypeHandle::NULL_HANDLE;
                        }
                        cloneIndex++;
                    }
                }
                loadedNode(nodeRoot, true);
            }
            catch (const std::runtime_error& ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", ex.what()));
                loadedNode(nodeRoot, false);
                throw ex;
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", ex));
                loadedNode(nodeRoot, false);
                throw ex;
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", ex));
                loadedNode(nodeRoot, false);
                throw ex;
            }
            catch (...) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: RESOLVE_NODE - {}", "UNKNOWN_ERROR"));
                loadedNode(nodeRoot, false);
                throw std::current_exception();
            }
            });

        return true;
    }

    pool::TypeHandle NodeBuilder::resolveNodeClone(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx->m_alive) return typeHandle;

        if (!nodeData.enabled) {
            return typeHandle;
        }

        const auto& repeat = nodeData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    if (!*m_ctx->m_alive) return typeHandle;

                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    typeHandle = resolveNodeCloneRepeat(
                        typeHandle,
                        rootId,
                        nodeRoot,
                        nodeData,
                        cloned,
                        cloneIndex,
                        tile,
                        tilePositionOffset);

                    if (!nodeRoot.base.shareType)
                        typeHandle = pool::TypeHandle::NULL_HANDLE;
                }
            }
        }

        return typeHandle;
    }

    pool::TypeHandle NodeBuilder::resolveNodeCloneRepeat(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset)
    {
        //if (!*m_ctx->m_alive) return typeHandle;

        if (!nodeData.enabled) {
            return typeHandle;
        }

        // NOTE KI overriding material in clones is *NOT* supported"
        if (!typeHandle) {
            const auto& repeat = nodeData.repeat;
            const bool hasTile = repeat.xCount > 1 || repeat.yCount > 1 || repeat.zCount > 1;

            std::string nameSuffix;
            if (cloned || hasTile) {
                nameSuffix = fmt::format(
                    "{}{}",
                    cloned ? fmt::format("clone_{}", cloneIndex) : "",
                    hasTile ? fmt::format("tile_{}x{}x{}", tile.x, tile.y, tile.z) : ""
                );
            }

            typeHandle = m_nodeTypeBuilder->createType(nodeData, nameSuffix);
            if (!typeHandle) return typeHandle;
        }

        if (!*m_ctx->m_alive) return typeHandle;

        auto [handle, state] = createNode(
            typeHandle,
            nodeData,
            cloneIndex,
            tile,
            nodeData.clonePositionOffset + tilePositionOffset);

        ki::node_id parentId;
        if (nodeData.parentBaseId.empty()) {
            parentId = rootId;
        }
        else {
            auto [id, _] = resolveNodeId(
                nodeData.parentBaseId,
                cloneIndex,
                tile);
            parentId = id;
        }

        ResolvedNode resolved{
            parentId,
            handle,
            nodeData,
            state,
        };

        addResolvedNode(resolved);

        return typeHandle;
    }

    std::pair<pool::NodeHandle, NodeState> NodeBuilder::createNode(
        pool::TypeHandle typeHandle,
        const NodeData& nodeData,
        const int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& positionOffset)
    {
        auto& l = *m_loaders;

        const auto* type = typeHandle.toType();

        ki::node_id nodeId{ 0 };
        std::string resolvedSID;
        {
            if (nodeData.baseId.empty()) {
                throw "ID missing";
            }

            auto [k, v] = resolveNodeId(nodeData.baseId, cloneIndex, tile);
            nodeId = k;
            resolvedSID = v;
        }

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);

        node->setName(resolvedSID);

        node->m_typeHandle = typeHandle;
        node->m_typeFlags = type->m_flags;
        node->m_layer = type->m_layer;

        {
            ki::node_id ignoredBy{ 0 };
            if (!nodeData.ignoredByBaseId.empty()) {
                auto [id, _] = resolveNodeId(
                    nodeData.ignoredByBaseId,
                    cloneIndex,
                    tile);
                ignoredBy = id;
            }
            node->m_ignoredBy = ignoredBy;
        }

        //node->setCloneIndex(cloneIndex);
        //node->setTile(tile);

        const glm::vec3 pos = nodeData.position + positionOffset;

        NodeState state;
        state.setPosition(pos);

        state.setRotation(util::degreesToQuat(nodeData.rotation));
        state.setScale(nodeData.scale);

        state.setPivot(nodeData.pivot.resolve(type));

        state.setFront(nodeData.front);

        {
            state.setBaseRotation(util::degreesToQuat(nodeData.baseRotation));
        }

        node->m_generator = l.m_generatorLoader.createGenerator(
            nodeData.generator,
            type,
            *m_loaders);

        return { handle, state };
    }
}
