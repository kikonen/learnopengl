#include "CompositeBuilder.h"

#include <string>
#include <vector>
#include <span>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/sid_format.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/sid.h"

#include "pool/IdGenerator.h"
#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/ResolvedNode.h"
#include "model/CompositeDefinition.h"
#include "model/NodeDefinition.h"
#include "model/DagSort.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace
{
    const std::string ROOT_ID{ "ROOT" };

    IdGenerator<ki::type_id> ID_GENERATOR;

    const NodeType* findNodeType(
       const std::string& baseId)
    {
        auto typeId = SID(baseId);
        return pool::TypeHandle::toType(typeId);
    }

    std::tuple<ki::node_id, std::string> resolveNodeId(
        ki::type_id typeId,
        const std::string& baseId)
    {
        const auto& assets = Assets::get();

        if (baseId.empty()) {
            const auto& typeName = SID_NAME(typeId);
            const auto& nodeName = fmt::format(
                "<{}>-{}",
                typeName, ID_GENERATOR.nextId());
            return { SID(nodeName), nodeName };
        }

        if (baseId == ROOT_ID) {
            return { assets.rootId, SID_NAME(assets.rootId) };
        }
        else {
            auto nodeId = SID(baseId);
            KI_DEBUG(fmt::format("SID: sid={}, key={}", nodeId, baseId));
            return { nodeId, baseId };
        }
    }

    const ki::node_id* findAlias(
        const std::string& aliasId,
        std::vector<std::pair<std::string, ki::node_id>>& aliases)
    {
        const auto& it = std::find_if(
            aliases.cbegin(),
            aliases.cend(),
            [&aliasId](const auto& e) { return e.first == aliasId; });
        return it != aliases.end() ? &(*it).second : nullptr;
    }
}

CompositeBuilder::CompositeBuilder(
    NodeRegistry& nodeRegistry)
    : m_nodeRegistry{ nodeRegistry }
{
}

CompositeBuilder::~CompositeBuilder() = default;

void CompositeBuilder::addResolvedNode(
    const ResolvedNode& resolved)
{
    m_resolvedNodes.push_back(resolved);
}

ki::node_id CompositeBuilder::build(
    const ki::node_id parentId,
    const NodeType* type,
    const CreateState& state)
{
    ki::node_id nodeId{ ki::StringID::nextID(type->getName()) };

    // NOTE KI cannot allow duplicate id
    if (pool::NodeHandle::toHandle(nodeId)) return 0;

    {
        const auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();

        node->m_typeHandle = type->toHandle();
        node->setName(ki::StringID::getName(nodeId));

        addResolvedNode({
            parentId,
            node->m_handle,
            false,
            state
        });
    }

    if (type->m_compositeDefinition) {
        std::vector<std::pair<std::string, ki::node_id>> aliases;
        build(nodeId, *type->m_compositeDefinition, aliases);
    }

    return nodeId;
}

bool CompositeBuilder::build(
    const ki::node_id parentId,
    const CompositeDefinition& compositeData,
    std::vector<std::pair<std::string, ki::node_id>> aliases)
{
    if (!compositeData.m_nodes) {
        KI_WARN_OUT(fmt::format("IGNORE: COMPOSITE_NO_NODES - composite={} parentId={}", compositeData.m_id, parentId));
        return false;
    }

    for (const auto& nodeData : *compositeData.m_nodes) {
        buildNode(parentId, nodeData, aliases, true);
    }

    return true;
}

void CompositeBuilder::attach()
{
    DagSort sorter;
    auto sorted = sorter.sort(m_resolvedNodes);

    for (auto* resolved : sorted) {
        m_nodeRegistry.attachNode(
            resolved->handle,
            resolved->parentId,
            resolved->state);
    }
}

void CompositeBuilder::buildNode(
    const ki::node_id parentId,
    const NodeDefinition& baseData,
    std::vector<std::pair<std::string, ki::node_id>>& aliases,
    bool root)
{
    try {
        if (!baseData.m_clones) {
             buildNodeClone(parentId, baseData, aliases);
        }
        else {
            for (const auto& cloneData : *baseData.m_clones) {
                buildNodeClone(parentId, cloneData, aliases);
            }
        }
    }
    catch (const std::runtime_error& ex) {
        KI_CRITICAL(fmt::format("COMPOSITE_ERROR: RESOLVE_NODE - {}", ex.what()));
    }
    catch (const std::string& ex) {
        KI_CRITICAL(fmt::format("COMPOSITE_ERROR: RESOLVE_NODE - {}", ex));
    }
    catch (const char* ex) {
        KI_CRITICAL(fmt::format("COMPOSITE_ERROR: RESOLVE_NODE - {}", ex));
    }
    catch (...) {
        KI_CRITICAL(fmt::format("COMPOSITE_ERROR: RESOLVE_NODE - {}", "UNKNOWN_ERROR"));
    }
}

void CompositeBuilder::buildNodeClone(
    const ki::node_id parentId,
    const NodeDefinition& cloneData,
    std::vector<std::pair<std::string, ki::node_id>>& aliases)
{
    if (!cloneData.m_enabled) {
        return;
    }

    const auto& repeat = cloneData.m_repeat;

    for (auto z = 0; z < repeat.m_zCount; z++) {
        for (auto y = 0; y < repeat.m_yCount; y++) {
            for (auto x = 0; x < repeat.m_xCount; x++) {
                const glm::vec3 tilePositionOffset{
                    x * repeat.m_xStep,
                    y * repeat.m_yStep,
                    z * repeat.m_zStep };

                buildNodeCloneRepeat(
                    parentId,
                    cloneData,
                    aliases,
                    tilePositionOffset);
            }
        }
    }
}

void CompositeBuilder::buildNodeCloneRepeat(
    const ki::node_id parentId,
    const NodeDefinition& cloneData,
    std::vector<std::pair<std::string, ki::node_id>>& aliases,
    const glm::vec3& tilePositionOffset)
{
    if (!cloneData.m_enabled) return;

    auto [handle, state] = createNode(
        cloneData,
        aliases,
        cloneData.m_clonePositionOffset + tilePositionOffset);

    if (!cloneData.m_aliasId.empty())
    {
        aliases.push_back({ cloneData.m_aliasId, handle.toId() });
    }

    ResolvedNode resolved{
        parentId,
        handle,
        cloneData.m_active,
        state,
    };

    addResolvedNode(resolved);

    if (cloneData.m_children) {
        for (const auto& childData : *cloneData.m_children) {
            buildNode(
                handle.toId(),
                childData,
                aliases,
                false);
        }
    }
}

std::pair<pool::NodeHandle, CreateState> CompositeBuilder::createNode(
    const NodeDefinition& nodeData,
    std::vector<std::pair<std::string, ki::node_id>>& aliases,
    const glm::vec3& positionOffset)
{
    const auto* type = pool::TypeHandle::toType(nodeData.m_typeId);
    if (!type) {
        throw fmt::format("type missing: node={}, type={}", nodeData.m_id, nodeData.m_typeId);
    }

    const auto [nodeId, resolvedSID] = resolveNodeId(
        nodeData.m_typeId,
        nodeData.m_id);

    KI_INFO_OUT("NODE_SID: " + resolvedSID);

    auto handle = pool::NodeHandle::allocate(nodeId);
    auto* node = handle.toNode();
    assert(node);

    node->setName(resolvedSID);

    node->m_typeHandle = type->toHandle();

    if (!nodeData.m_ignoredById.empty())
    {
        auto* ignoredBy = findAlias(nodeData.m_ignoredById, aliases);
        if (!ignoredBy) {
            throw fmt::format("ignored_by missing: node={}, ignored_by={}", nodeData.m_id, nodeData.m_ignoredById);
        }
        node->m_ignoredBy = *ignoredBy;
    }

    const glm::vec3 pos = nodeData.m_position + positionOffset;

    CreateState state;
    state.m_position = pos;
    state.m_scale = nodeData.m_scale;
    state.m_rotation = util::degreesToQuat(nodeData.m_rotation);
    state.m_tilingX = nodeData.m_tiling.x;
    state.m_tilingY = nodeData.m_tiling.y;

    if (type->m_compositeDefinition) {
        CompositeBuilder builder{ NodeRegistry::get() };
        if (builder.build(nodeId, *type->m_compositeDefinition, aliases)) {
            for (auto& resolvedNode : builder.getResolvedNodes()) {
                addResolvedNode(resolvedNode);
            }
        }
    }

    return { handle, state };
}
