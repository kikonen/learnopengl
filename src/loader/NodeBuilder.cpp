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

#include "loader_util.h"

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
                    resolveNodeClone(rootId, nodeRoot, nodeRoot.base, false, 0);
                }
                else {
                    int cloneIndex = 0;
                    for (auto& cloneData : nodeRoot.clones) {
                        if (!*m_ctx->m_alive) return;
                        resolveNodeClone(rootId, nodeRoot, cloneData, true, cloneIndex);
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

    void NodeBuilder::resolveNodeClone(
        const ki::node_id rootId,
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx->m_alive) return;

        if (!nodeData.enabled) {
            return;
        }

        const auto& repeat = nodeData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    if (!*m_ctx->m_alive) return;

                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    resolveNodeCloneRepeat(
                        rootId,
                        nodeRoot,
                        nodeData,
                        cloned,
                        cloneIndex,
                        tile,
                        tilePositionOffset);
                }
            }
        }
    }

    void NodeBuilder::resolveNodeCloneRepeat(
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
            return;
        }

        if (!*m_ctx->m_alive) return;

        auto [handle, state] = createNode(
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
                nodeData.typeId,
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
    }

    std::pair<pool::NodeHandle, CreateState> NodeBuilder::createNode(
        const NodeData& nodeData,
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

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);

        node->setName(resolvedSID);

        node->m_typeHandle = type->toHandle();

        {
            ki::node_id ignoredBy{ 0 };
            if (!nodeData.ignoredByBaseId.empty()) {
                auto [id, _] = resolveNodeId(
                    nodeData.typeId,
                    nodeData.ignoredByBaseId,
                    cloneIndex,
                    tile);
                ignoredBy = id;
            }
            node->m_ignoredBy = ignoredBy;
        }

        const glm::vec3 pos = nodeData.position + positionOffset;

        CreateState state;
        state.m_position = pos;
        state.m_scale = nodeData.scale;
        state.m_rotation = util::degreesToQuat(nodeData.rotation);

        node->m_generator = l.m_generatorLoader.createGenerator(
            nodeData.generator,
            *m_loaders);

        return { handle, state };
    }
}
