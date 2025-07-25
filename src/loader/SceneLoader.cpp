#include "SceneLoader.h"

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

#include "material/Material.h"
#include "material/MaterialUpdater.h"
#include "material/MaterialRegistry.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "decal/DecalRegistry.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"
#include "model/NodeType.h"
#include "model/ResolvedNode.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"

#include <engine/AsyncLoader.h>

#include "model/ResolvedNode.h"
#include "model/DagSort.h"

#include "Context.h"
#include "Loaders.h"
#include "NodeTypeData.h"
#include "NodeData.h"
#include "CompositeData.h"
#include "DecalData.h"
#include "ScriptData.h"

#include "NodeTypeBuilder.h"
#include "NodeBuilder.h"

#include "converter/YamlConverter.h"

#include "loader/document.h"
#include "loader_util.h"


namespace loader {
    SceneLoader::SceneLoader(
        std::shared_ptr<Context> ctx)
        : BaseLoader(ctx),
        m_loaders{ std::make_shared<Loaders>(ctx) },
        m_nodeTypeBuilder{ std::make_unique<NodeTypeBuilder>(ctx, m_loaders) },
        m_nodeBuilder{ std::make_unique<NodeBuilder>(this) },
        m_meta{ std::make_unique<MetaData>() },
        m_root{ std::make_unique<RootData>() },
        m_skybox{ std::make_unique<SkyboxData>() },
        m_scriptSystemData{ std::make_unique<ScriptSystemData>() }
    {
        // NOTE KI white causes least unexpectedly tinted results
        m_defaultMaterial = std::make_unique<Material>(Material::createMaterial(BasicMaterial::white));
    }

    SceneLoader::~SceneLoader()
    {
        //KI_INFO(fmt::format("SCENE_FILE: delete - ctx={}", m_ctx->str()));
    }

    void SceneLoader::destroy()
    {}

    bool SceneLoader::isRunning()
    {
        std::lock_guard lock(m_ready_lock);
        return m_runningCount > 0 || m_pendingCount > 0;
    }

    void SceneLoader::prepare(
        std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = registry->m_dispatcherWorker;

        m_loaders->prepare(registry);
    }

    void SceneLoader::load()
    {
        if (!util::fileExists(m_ctx->m_fullPath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", m_ctx->str()) };
        }

        std::lock_guard lock(m_ready_lock);
        m_runningCount++;

        m_ctx->m_asyncLoader->addLoader(m_ctx->m_alive, [this]() {
            try {
                auto& l = *m_loaders;

                YamlConverter converter;
                auto doc = converter.load(m_ctx->m_fullPath);

                loadMeta(doc.findNode("meta"), *m_meta);

                l.m_skyboxLoader.loadSkybox(doc.findNode("skybox"), *m_skybox);

                l.m_rootLoader.loadRoot(doc.findNode("root"), *m_root);
                l.m_scriptLoader.loadScriptSystem(doc.findNode("script"), *m_scriptSystemData);
                l.m_materialUpdaterLoader.loadMaterialUpdaters(
                    doc.findNode("material_updaters"),
                    m_materialUpdaters,
                    *m_loaders);

                l.m_nodeTypeLoader.loadNodeTypes(
                    doc.findNode("types"),
                    m_nodeTypes,
                    l);

                l.m_compositeLoader.loadComposites(
                    doc.findNode("composites"),
                    m_composites,
                    l);

                l.m_particleLoader.loadParticles(
                    doc.findNode("particles"),
                    m_particles,
                    l);

                l.m_nodeLoader.loadNodes(
                    doc.findNode("nodes"),
                    m_nodes,
                    l);

                l.m_decalLoader.loadDecals(
                    doc.findNode("decals"),
                    m_decals,
                    l);

                validate(*m_root);
                attach(*m_root);
            }
            catch (const std::runtime_error& ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: LOAD - {}", ex.what()));
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

            std::lock_guard lock(m_ready_lock);
            m_runningCount--;
        });
    }

    void SceneLoader::loadedNode(
        const NodeData& nodeData,
        bool success)
    {
        std::lock_guard lock(m_ready_lock);

        m_pendingCount--;

        KI_INFO_OUT(fmt::format(
            "LOADED: node={}, success={}, pending={}",
            nodeData.name, success, m_pendingCount));

        if (m_pendingCount > 0) return;

        // NOTE KI event will be put queue *AFTER* node attach events
        // => should they should be fully attached in scene at this point
        // => worker will trigger event into UI thread after processing all updates

        m_ctx->m_asyncLoader->addLoader(m_ctx->m_alive, [this]() {
            try {
                attachResolvedNodes(m_nodeBuilder->getResolvedNodes());
                notifySceneLoaded();
            }
            catch (const std::runtime_error& ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: LOADED_NODE - {}", ex.what()));
                throw ex;
            }
            catch (const std::string& ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: LOADED_NODE - {}", ex));
                throw ex;
            }
            catch (const char* ex) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: LOADED_NODE - {}", ex));
                throw ex;
            }
            catch (...) {
                KI_CRITICAL(fmt::format("SCENE_ERROR: LOADED_NODE - {}", "UNKNOWN_ERROR"));
                throw std::current_exception();
            }
        });
    }

    void SceneLoader::notifySceneLoaded()
    {
        {
            event::Event evt { event::Type::scene_loaded };
            m_dispatcher->send(evt);
        }
    }

    void SceneLoader::attach(
        const RootData& root)
    {
        auto& l = *m_loaders;

        l.m_rootLoader.attachRoot(root, *m_scriptSystemData, *m_loaders);
        l.m_skyboxLoader.attachSkybox(root.rootId, *m_skybox);

        {
            auto updaters = l.m_materialUpdaterLoader.createMaterialUpdaters(
                m_materialUpdaters,
                *m_loaders);

            auto& materialRegistry = MaterialRegistry::get();
            for (auto&& updater : updaters) {
                materialRegistry.addMaterialUpdater(std::move(updater));
            }
        }

        {
            const auto& decals = l.m_decalLoader.createDecals(
                m_decals,
                l);

            for (const auto& decal : decals) {
                decal::DecalRegistry::get().addDecal(decal);
            }
        }

        {
            m_nodeTypeBuilder->createTypes(
                m_nodeTypes,
                m_composites,
                m_particles);
        }

        {
            std::lock_guard lock(m_ready_lock);

            m_pendingCount = 0;
            for (const auto& node : m_nodes) {
                if (m_nodeBuilder->resolveNode(root.rootId, node, m_ctx, m_loaders)) {
                    m_pendingCount++;
                    KI_INFO_OUT(fmt::format("START: node={}, pending={}", node.name, m_pendingCount));
                }
            }

            KI_INFO_OUT(fmt::format("TOTAL: pending={}", m_pendingCount));
        }
    }

    void SceneLoader::attachResolvedNodes(
        std::vector<ResolvedNode>& resolvedNodes)
    {
        auto& l = *m_loaders;

        DagSort sorter;
        auto sorted = sorter.sort(resolvedNodes);

        for (auto* resolved : sorted) {
            if (!*m_ctx->m_alive) return;
            attachResolvedNode(*resolved);
        }
    }

    void SceneLoader::attachResolvedNode(
        const ResolvedNode& resolved)
    {
        auto& nodeHandle = resolved.handle;

        {
            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body = {
                .state = resolved.state,
            };
            evt.body.node = {
                .target = nodeHandle.toId(),
                .parentId = resolved.parentId,
            };
            assert(evt.body.node.target > 1);
            m_dispatcher->send(evt);
        }

        if (resolved.active) {
            event::Event evt { event::Type::node_activate };
            evt.body.node = {
                .target = nodeHandle.toId(),
            };
            m_dispatcher->send(evt);
        }

        // TODO KI default camera logic
        //if (nodeData.camera.isDefault) {
        //    event::Event evt { event::Type::camera_activate };
        //    evt.body.node = {
        //        .target = handle.toId(),
        //    };
        //    m_dispatcherView->send(evt);
        //}

        // try anim event
        //if (!node.isRoot && !type->m_flags.water && !type->m_flags.tessellation && !type->m_flags.noShadow)
        //if (nodeData.desc == "Cow")
        //{
        //    event::Event evt { event::Type::animate_rotate };
        //    evt.body.animate = {
        //        .target = handle.toId(),
        //        .duration = 20,
        //        .data = { 0, 1.f, 0 },
        //        .data2 = { 360.f, 0.f, 0.f },
        //    };
        //    m_dispatcher->send(evt);
        //}
    }

    void SceneLoader::loadMeta(
        const loader::DocNode& node,
        MetaData& data) const
    {
        data.name = "<noname>";
        //data.modelsDir = assets.modelsDir;

        for (const auto& pair : node.getNodes()) {
            const std::string& k = pair.getName();
            const loader::DocNode& v = pair.getNode();

            if (k == "name") {
                data.name = "";// readString(v);
            }
            //else if (k == "assetsDir") {
            //    data.assetsDir = readString(v);
            //}
            //else if (k == "modelsDir") {
            //    data.modelsDir = readString(v);
            //}
            else {
                reportUnknown("meta_entry", k, v);
            }
        }
    }

    void SceneLoader::validate(
        const RootData& root)
    {
        std::map<ki::node_id, std::string> collectedIds;
        int pass1Errors = 0;
        int pass2Errors = 0;

        for (const auto& node : m_nodes) {
            validateNode(root.rootId, node, 0, pass1Errors, collectedIds);
        }

        for (const auto& node : m_nodes) {
            validateNode(root.rootId, node, 1, pass2Errors, collectedIds);
        }

        if (pass1Errors > 0 || pass2Errors > 0) {
            auto msg = fmt::format("SCENE_ERROR: VALIDATE: FAILED - pass1={}, pass2={}", pass1Errors, pass2Errors);
            KI_CRITICAL(msg);
            throw std::runtime_error{ msg };
        }
    }

    void SceneLoader::validateNode(
        const ki::node_id rootId,
        const NodeData& baseData,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!baseData.clones) {
            validateNodeClone(rootId, baseData, baseData, false, pass, errorCount, collectedIds);
        }
        else {
            for (auto& cloneData : *baseData.clones) {
                validateNodeClone(rootId, baseData, cloneData, true, pass, errorCount, collectedIds);
            }
        }
    }

    void SceneLoader::validateNodeClone(
        const ki::node_id rootId,
        const NodeData& baseData,
        const NodeData& cloneData,
        bool cloned,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!cloneData.enabled) return;

        const auto& repeat = cloneData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    validateNodeCloneRepeat(
                        rootId,
                        baseData,
                        cloneData,
                        cloned,
                        tile,
                        tilePositionOffset,
                        pass,
                        errorCount,
                        collectedIds);
                }
            }
        }
    }

    void SceneLoader::validateNodeCloneRepeat(
        const ki::node_id rootId,
        const NodeData& baseData,
        const NodeData& cloneData,
        bool cloned,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!cloneData.enabled) return;

        if (pass == 0) {
            ki::node_id sid;
            std::string resolvedSID;
            {
                bool automatic = cloneData.baseId.m_path == baseData.baseId.m_path;
                auto [k, v] = resolveNodeId(
                    cloneData.typeId,
                    cloneData.baseId);
                sid = k;
                resolvedSID = v;
            }

            if (const auto& it = collectedIds.find(sid); it != collectedIds.end()) {
                auto msg = fmt::format("SCENE_ERROR: SID CONFLICT: {} = {} (WAS: {})", sid, resolvedSID, it->second);
                KI_CRITICAL(msg);
                errorCount++;
            }
            collectedIds[sid] = resolvedSID;

            KI_INFO_OUT("NODE_SID: " + resolvedSID);
        }

        if (pass == 1) {
            // NOTE KI parentId can be *MISSING*
            // but it cannot be duplicate
            if (!cloneData.parentBaseId.empty()) {
                auto [sid, resolvedSID] = resolveNodeId(
                    cloneData.typeId,
                    cloneData.parentBaseId);

                // TODO KI validate missing
                if (collectedIds.find(sid) == collectedIds.end()) {
                    auto msg = fmt::format("SCENE_ERROR: PARENT SID MISSING: {} = {}", sid, resolvedSID);
                    KI_CRITICAL(msg);
                    errorCount++;
                }
            }
        }
    }
}
