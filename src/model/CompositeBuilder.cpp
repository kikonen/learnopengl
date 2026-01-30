#include "CompositeBuilder.h"

#include <string>
#include <vector>
#include <span>

#include <fmt/format.h>

#include "util/util.h"
#include "util/file.h"
#include "util/thread.h"
#include "util/sid_format.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/sid.h"

#include "pool/IdGenerator.h"
#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"

#include "event/Event.h"
#include "event/Dispatcher.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/ResolvedNode.h"
#include "model/CompositeDefinition.h"
#include "model/NodeDefinition.h"

#include "util/DagSort.h"
#include "util/DagSort_impl.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


namespace
{
    const std::string ROOT_ID{ "ROOT" };

    using t_dag_item = util::DagItem<ki::node_id, model::ResolvedNode>;

    IdGenerator<ki::type_id> ID_GENERATOR;

    const model::NodeType* findNodeType(
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
            KI_DEBUG(fmt::format("MODEL::SID: sid={}, key={}", nodeId, baseId));
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

namespace model
{
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

    pool::NodeHandle CompositeBuilder::build(
        const ki::node_id parentId,
        const ki::socket_id socketId,
        const model::NodeType* type,
        const CreateState& state)
    {
        ki::node_id nodeId{ ki::StringID::nextID(
            fmt::format("<{}>", type->getName())) };

        // NOTE KI cannot allow duplicate id
        if (pool::NodeHandle::toHandle(nodeId)) return pool::NodeHandle::NULL_HANDLE;

        pool::NodeHandle handle;
        {
            handle = pool::NodeHandle::allocate(nodeId);
            auto* node = handle.toNode();

            node->m_typeHandle = type->toHandle();
            node->setName(SID_NAME(nodeId));

            addResolvedNode({
                parentId,
                socketId,
                node->m_handle,
                false,
                state
                });
        }

        if (type->m_compositeDefinition) {
            std::vector<std::pair<std::string, ki::node_id>> aliases;
            build(nodeId, *type->m_compositeDefinition, aliases);
        }

        return handle;
    }

    bool CompositeBuilder::build(
        const ki::node_id parentId,
        const model::CompositeDefinition& compositeData,
        std::vector<std::pair<std::string, ki::node_id>> aliases)
    {
        if (!compositeData.m_nodes) {
            KI_WARN_OUT(fmt::format("MODEL::IGNORE: COMPOSITE_NO_NODES - composite={} parentId={}", compositeData.m_id, parentId));
            return false;
        }

        for (const auto& nodeData : *compositeData.m_nodes) {
            buildNode(parentId, nodeData, aliases, true);
        }

        return true;
    }

    pool::NodeHandle CompositeBuilder::attach()
    {
        ASSERT_WT();

        std::vector<t_dag_item> sorted;
        {
            std::vector<t_dag_item> items;
            items.reserve(m_resolvedNodes.size());
            for (auto& node : m_resolvedNodes) {
                items.push_back({ node.parentId, node.handle.toId(), &node });
            }

            util::DagSort<ki::node_id, ResolvedNode> sorter;
            sorted = sorter.sort(items);
        }

        for (auto& item : sorted) {
            auto* resolved = item.data;
            m_nodeRegistry.attachNode(
                resolved->handle,
                resolved->parentId,
                resolved->socketId,
                resolved->state);
        }

        return sorted[0].data->handle;
    }

    pool::NodeHandle CompositeBuilder::asyncAttach(Registry* registry)
    {
        std::vector<t_dag_item> sorted;
        {
            std::vector<t_dag_item> items;
            items.reserve(m_resolvedNodes.size());
            for (auto& node : m_resolvedNodes) {
                items.push_back({ node.parentId, node.handle.toId(), &node });
            }

            util::DagSort<ki::node_id, ResolvedNode> sorter;
            sorted = sorter.sort(items);
        }

        for (auto& item : sorted) {
            asyncAttach(*item.data, registry);
        }

        return sorted[0].data->handle;
    }

    pool::NodeHandle CompositeBuilder::asyncAttach(
        const ResolvedNode& resolved,
        Registry* registry)
    {
        auto& nodeHandle = resolved.handle;

        {
            event::Event evt{ event::Type::node_add };
            auto* att = evt.attach();
            att->nodeEntry = {
                .state = resolved.state,
            };
            evt.body.node = {
                .target = nodeHandle.toId(),
                .parentId = resolved.parentId,
                .socketId = resolved.socketId,
            };
            assert(evt.body.node.target > 1);
            registry->m_dispatcherWorker->send(evt);
        }

        return nodeHandle;
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
            KI_CRITICAL(fmt::format("MODEL::COMPOSITE_ERROR: RESOLVE_NODE - {}", ex.what()));
        }
        catch (const std::exception& ex) {
            KI_CRITICAL(fmt::format("MODEL::COMPOSITE_ERROR: RESOLVE_NODE - {}", ex.what()));
        }
        catch (const std::string& ex) {
            KI_CRITICAL(fmt::format("MODEL::COMPOSITE_ERROR: RESOLVE_NODE - {}", ex));
        }
        catch (const char* ex) {
            KI_CRITICAL(fmt::format("MODEL::COMPOSITE_ERROR: RESOLVE_NODE - {}", ex));
        }
        catch (...) {
            KI_CRITICAL(fmt::format("MODEL::COMPOSITE_ERROR: RESOLVE_NODE - {}", "UNKNOWN_ERROR"));
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
        const ki::node_id defaultParentId,
        const NodeDefinition& cloneData,
        std::vector<std::pair<std::string, ki::node_id>>& aliases,
        const glm::vec3& tilePositionOffset)
    {
        if (!cloneData.m_enabled) return;

        auto [handle, state] = createNode(
            cloneData,
            aliases,
            cloneData.m_clonePositionOffset + tilePositionOffset);

        ki::node_id parentId = defaultParentId;
        if (cloneData.m_parentId.empty()) {
            parentId = defaultParentId;
        }
        else {
            auto [id, _] = resolveNodeId(
                cloneData.m_typeId,
                cloneData.m_parentId);
            parentId = id;
        }

        ki::socket_id socketId = SID(cloneData.m_socketId);

        if (!cloneData.m_aliasId.empty())
        {
            aliases.push_back({ cloneData.m_aliasId, handle.toId() });
        }

        ResolvedNode resolved{
            parentId,
            socketId,
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
            throw fmt::format(
                "type missing: node={}, type={}",
                nodeData.m_id, SID_NAME(nodeData.m_typeId));
        }

        const auto [nodeId, resolvedSID] = resolveNodeId(
            nodeData.m_typeId,
            nodeData.m_id);

        KI_INFO("MODEL::NODE_SID: " + resolvedSID);

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);

        const ki::socket_id socketId = SID(nodeData.m_socketId);

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

        model::CreateState state;
        state.m_position = pos;
        state.m_scale = nodeData.m_scale;
        state.m_rotation = util::degreesToQuat(nodeData.m_rotation);
        state.m_tilingX = nodeData.m_tiling.x;
        state.m_tilingY = nodeData.m_tiling.y;
        state.m_tagId = SID(nodeData.m_tagId);

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
}
