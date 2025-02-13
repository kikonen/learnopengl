#include "SceneLoader.h"

#include <string>
#include <vector>
#include <fstream>
#include <string>
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

#include "shader/Program.h"
#include "shader/Shader.h"
#include "shader/ProgramRegistry.h"

#include "mesh/mesh_util.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"
#include "mesh/TextMesh.h"
#include "mesh/NonVaoMesh.h"

#include "component/Light.h"
#include "component/CameraComponent.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"
#include "model/NodeType.h"

#include "animation/AnimationLoader.h"
#include "animation/RigContainer.h"
#include "animation/RigSocket.h"

#include "event/Dispatcher.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/MeshTypeRegistry.h"

#include <engine/AsyncLoader.h>

#include "DagSort.h"

#include "Loaders.h"
#include "NodeRoot.h"
#include "ResolvedNode.h"
#include "DecalData.h"

#include "converter/YamlConverter.h"

#include "loader/document.h"
#include "loader_util.h"

namespace {
    const std::string QUAD_MESH_NAME{ "quad" };
}

namespace loader {
    SceneLoader::SceneLoader(
        Context ctx)
        : BaseLoader(ctx),
        m_loaders{ std::make_unique<Loaders>(ctx) },
        m_meta{ std::make_unique<MetaData>() },
        m_root{ std::make_unique<RootData>() },
        m_skybox{ std::make_unique<SkyboxData>() },
        m_scriptEngineData{ std::make_unique<ScriptEngineData>() }
    {
        // NOTE KI white causes least unexpectedly tinted results
        m_defaultMaterial = std::make_unique<Material>(Material::createMaterial(BasicMaterial::white));
    }

    SceneLoader::~SceneLoader()
    {
        //KI_INFO(fmt::format("SCENE_FILE: delete - ctx={}", m_ctx.str()));
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
        if (!util::fileExists(m_ctx.m_fullPath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", m_ctx.str()) };
        }

        std::lock_guard lock(m_ready_lock);
        m_runningCount++;

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this]() {
            try {
                auto& l = *m_loaders;

                YamlConverter converter;
                auto doc = converter.load(m_ctx.m_fullPath);

                loadMeta(doc.findNode("meta"), *m_meta);

                l.m_skyboxLoader.loadSkybox(doc.findNode("skybox"), *m_skybox);

                l.m_rootLoader.loadRoot(doc.findNode("root"), *m_root);
                l.m_scriptLoader.loadScriptEngine(doc.findNode("script"), *m_scriptEngineData);
                l.m_materialUpdaterLoader.loadMaterialUpdaters(
                    doc.findNode("material_updaters"),
                    m_materialUpdaters,
                    *m_loaders);

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
        const NodeRoot& nodeRoot,
        bool success)
    {
        std::lock_guard lock(m_ready_lock);

        m_pendingCount--;

        KI_INFO_OUT(fmt::format(
            "LOADED: node={}, success={}, pending={}",
            nodeRoot.base.name, success, m_pendingCount));

        if (m_pendingCount > 0) return;

        // NOTE KI event will be put queue *AFTER* node attach events
        // => should they should be fully attached in scene at this point
        // => worker will trigger event into UI thread after processing all updates

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this]() {
            try {
                attachResolvedNodes(m_resolvedNodes);
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

        l.m_rootLoader.attachRoot(root);
        l.m_scriptLoader.createScriptEngine(*m_scriptEngineData);

        l.m_skyboxLoader.attachSkybox(root.rootId, *m_skybox);

        l.m_materialUpdaterLoader.createMaterialUpdaters(
            m_materialUpdaters,
            *m_loaders);

        l.m_decalLoader.createDecals(
            m_decals,
            l);

        {
            std::lock_guard lock(m_ready_lock);

            m_pendingCount = 0;
            for (const auto& node : m_nodes) {
                if (resolveNode(root.rootId, node)) {
                    m_pendingCount++;
                    KI_INFO_OUT(fmt::format("START: node={}, pending={}", node.base.name, m_pendingCount));
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
            if (!*m_ctx.m_alive) return;
            if (resolved->scriptIds.empty()) continue;

            m_loaders->m_scriptLoader.bindNodeScripts(
                resolved->handle,
                resolved->scriptIds);
        }

        for (auto* resolved : sorted) {
            if (!*m_ctx.m_alive) return;
            attachResolvedNode(*resolved);
        }
    }

    void SceneLoader::attachResolvedNode(
        const ResolvedNode& resolved)
    {
        auto& l = *m_loaders;
        auto& handle = resolved.handle;
        auto& nodeData = resolved.data;

        {
            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body = {
                .state = resolved.state,
                .physics = {
                    .body = nodeData.physics.body,
                    .geom = nodeData.physics.geom,
                    .enabled = nodeData.physics.enabled,
                    .update = nodeData.physics.update
                },
            };
            evt.body.node = {
                .target = handle.toId(),
                .parentId = resolved.parentId,
            };
            assert(evt.body.node.target > 1);
            m_dispatcher->send(evt);
        }

        if (nodeData.active) {
            event::Event evt { event::Type::node_activate };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        if (nodeData.selected) {
            event::Event evt { event::Type::node_select };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        if (nodeData.camera.isDefault) {
            event::Event evt { event::Type::camera_activate };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        for (auto& controllerData : nodeData.controllers) {
            if (!controllerData.enabled) continue;

            auto* controller = l.m_controllerLoader.createController(controllerData, handle);

            event::Event evt { event::Type::controller_add };
            evt.body.control = {
                .target = handle.toId(),
                .controller = controller
            };
            m_dispatcher->send(evt);
        }

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

    void SceneLoader::addResolvedNode(
        const ResolvedNode& resolved)
    {
        {
            std::lock_guard lock(m_ready_lock);
            m_resolvedNodes.push_back(resolved);
        }
    }

    bool SceneLoader::resolveNode(
        const ki::node_id rootId,
        const NodeRoot& nodeRoot)
    {
        if (!nodeRoot.base.enabled) {
            return false;
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this, rootId, &nodeRoot]() {
            try {
                if (nodeRoot.clones.empty()) {
                    pool::TypeHandle typeHandle{};
                    resolveNodeClone(typeHandle, rootId, nodeRoot, nodeRoot.base, false, 0);
                }
                else {
                    pool::TypeHandle typeHandle{};

                    int cloneIndex = 0;
                    for (auto& cloneData : nodeRoot.clones) {
                        if (!*m_ctx.m_alive) return;
                        typeHandle = resolveNodeClone(typeHandle, rootId, nodeRoot, cloneData, true, cloneIndex);
                        if (!nodeRoot.base.cloneMesh) {
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

    pool::TypeHandle SceneLoader::resolveNodeClone(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx.m_alive) return typeHandle;

        if (!nodeData.enabled) {
            return typeHandle;
        }

        const auto& repeat = nodeData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    if (!*m_ctx.m_alive) return typeHandle;

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

                    if (!nodeRoot.base.cloneMesh)
                        typeHandle = pool::TypeHandle::NULL_HANDLE;
                }
            }
        }

        return typeHandle;
    }

    pool::TypeHandle SceneLoader::resolveNodeCloneRepeat(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset)
    {
        if (!*m_ctx.m_alive) return typeHandle;

        if (!nodeData.enabled) {
            return typeHandle;
        }

        // NOTE KI overriding material in clones is *NOT* supported"
        if (!typeHandle) {
            typeHandle = createType(
                nodeData,
                tile);
            if (!typeHandle) return typeHandle;
        }

        if (!*m_ctx.m_alive) return typeHandle;

        auto [handle, state] = createNode(
            typeHandle, rootId, nodeData,
            cloned, cloneIndex, tile,
            nodeData.clonePositionOffset,
            tilePositionOffset);

        ki::node_id parentId;
        if (nodeData.parentBaseId.empty()) {
            parentId = rootId;
        }
        else {
            auto [id, _] = resolveId(
                nodeData.parentBaseId,
                cloneIndex,
                tile,
                false);
            parentId = id;
        }

        const auto scriptIds = m_loaders->m_scriptLoader.createScripts(
            handle,
            nodeData.scripts);

        ResolvedNode resolved{
            parentId,
            handle,
            nodeData,
            state,
            scriptIds
        };

        addResolvedNode(resolved);

        return typeHandle;
    }

    const pool::TypeHandle SceneLoader::createType(
        const NodeData& nodeData,
        const glm::uvec3& tile)
    {
        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();

        type->setName(nodeData.baseId.m_path.empty() ? nodeData.name : nodeData.baseId.m_path);
        type->m_layer = nodeData.layer;

        assignTypeFlags(nodeData, typeHandle);

        if (nodeData.type == NodeType::origo) {
            type->m_flags.invisible = true;
            type->m_nodeType = NodeType::origo;
        } else
        {
            resolveMeshes(type, nodeData, tile);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (nodeData.type != NodeType::container) {
                if (!type->hasMesh()) {
                    KI_WARN(fmt::format(
                        "SCENE_FILEIGNORE: NO_MESH id={} ({})",
                        nodeData.baseId, nodeData.desc));
                    return pool::TypeHandle::NULL_HANDLE;
                }
            }
        }

        {
            type->setCustomMaterial(
                m_loaders->m_customMaterialLoader.createCustomMaterial(
                    nodeData.customMaterial,
                    *m_loaders));
        }

        return typeHandle;
    }

    void SceneLoader::resolveMaterials(
        mesh::MeshType* type,
        mesh::LodMesh& lodMesh,
        const NodeData& nodeData,
        const MeshData& meshData,
        const LodData* lodData)
    {
        auto* lodMaterial = lodMesh.modifyMaterial();
        if (!lodMaterial) return;

        auto& material = *lodMaterial;

        if (!material.inmutable)
        {
            auto& l = *m_loaders;

            // ASSIGN
            for (auto& materialData : meshData.materials) {
                const auto& alias = materialData.aliasName;
                const auto& name = materialData.materialName;

                if (name == material.m_name || alias == material.m_name || alias == "*")
                {
                    KI_INFO_OUT(fmt::format(
                        "MAT_ASSIGN: model={}, mesh={}, material={}, name={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        name,
                        alias));

                    material.assign(materialData.material);
                }
            }

            // NOTE KI settings from mesh to material *AFTER* assign, *BEFORE* modify
            if (meshData.defaultPrograms) {
                material.m_defaultPrograms = true;
            }

            {
                for (const auto& srcIt : nodeData.programs) {
                    const auto& dstIt = material.m_programNames.find(srcIt.first);
                    if (dstIt == material.m_programNames.end()) {
                        material.m_programNames[srcIt.first] = srcIt.second;
                    }
                }

                for (const auto& srcIt : meshData.programs) {
                    const auto& dstIt = material.m_programNames.find(srcIt.first);
                    if (dstIt == material.m_programNames.end()) {
                        material.m_programNames[srcIt.first] = srcIt.second;
                    }
                }
            }

            // MODIFY - BASE
            for (auto& materialData : meshData.materialModifiers) {
                const auto& alias = materialData.aliasName;
                const auto& name = materialData.materialName;

                if (name == material.m_name || alias == material.m_name || alias == "*")
                {
                    KI_INFO_OUT(fmt::format(
                        "MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
                        type->getName(),
                        lodMesh.getMeshName(),
                        material.m_name,
                        name,
                        alias));

                    l.m_materialLoader.modifyMaterial(material, materialData);
                }
            }

            // MODIFY - LOD
            if (lodData) {
                for (auto& materialData : lodData->materialModifiers) {
                    const auto& alias = materialData.aliasName;
                    const auto& name = materialData.materialName;

                    if (name == material.m_name || alias == material.m_name || alias == "*")
                    {
                        KI_INFO_OUT(fmt::format(
                            "MAT_MODIFY: model={}, mesh={}, material={}, name={}, alias={}",
                            type->getName(),
                            lodMesh.getMeshName(),
                            material.m_name,
                            name,
                            alias));

                        l.m_materialLoader.modifyMaterial(material, materialData);
                    }
                }
            }
        }

        m_loaders->m_materialLoader.resolveMaterial(lodMesh.m_flags, material);
    }

    void SceneLoader::resolveMeshes(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const glm::uvec3& tile)
    {
        uint16_t index = 0;
        for (const auto& meshData : nodeData.meshes) {
            if (!meshData.enabled) continue;
            resolveMesh(type, nodeData, meshData, tile, index);
            index++;
        }

        // NOTE KI ensure volume is containing all meshes
        type->prepareVolume();
    }

    void SceneLoader::resolveMesh(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        const glm::uvec3& tile,
        int index)
    {
        const auto& assets = Assets::get();

        size_t startIndex = type->m_lodMeshes->size();
        size_t meshCount = 0;

        // NOTE KI materials MUST be resolved before loading mesh
        if (nodeData.type == NodeType::model) {
            type->m_nodeType = NodeType::model;
            meshCount += resolveModelMesh(type, nodeData, meshData, tile, index);

            KI_INFO(fmt::format(
                "SCENE_FILE MESH: id={}, desc={}, type={}",
                nodeData.baseId, nodeData.desc, type->str()));
        }
        else if (nodeData.type == NodeType::text) {
            type->m_nodeType = NodeType::text;
            auto mesh = std::make_unique<mesh::TextMesh>();

            if (!mesh->getMaterial()) {
                const auto& material = Material::createMaterial(BasicMaterial::yellow);
                mesh->setMaterial(&material);
            }

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(std::move(mesh), true);
            type->addLodMesh(std::move(lodMesh));
            meshCount++;
        }
        else if (nodeData.type == NodeType::terrain) {
            // NOTE KI handled via container + generator
            type->m_nodeType = NodeType::terrain;
            throw std::runtime_error("Terrain not supported currently");
        }
        else if (nodeData.type == NodeType::container) {
            // NOTE KI generator takes care of actual work
            type->m_nodeType = NodeType::container;
            type->m_flags.invisible = true;
        }
        else {
            // NOTE KI root/origo/unknown; don't render, just keep it in hierarchy
            type->m_nodeType = NodeType::origo;
            type->m_flags.invisible = true;
        }

        // Resolve materials for newly added meshes
        if (meshCount > 0) {
            const auto& span = std::span{ *type->m_lodMeshes.get() }.subspan(startIndex, meshCount);
            for (auto& lodMesh : span) {
                resolveLodMesh(type, nodeData, meshData, lodMesh);

                auto* mesh = lodMesh.getMesh<mesh::Mesh>();
                const auto rig = mesh ? mesh->getRigContainer() : nullptr;
                if (rig) {
                    rig->dump();
                }
            }
        }
    }

    int SceneLoader::resolveModelMesh(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        const glm::uvec3& tile,
        int index)
    {
        const auto& assets = Assets::get();
        int meshCount = 0;

        switch (meshData.type) {
        case MeshDataType::mesh: {
            auto future = ModelRegistry::get().getMeshSet(
                meshData.id,
                assets.modelsDir,
                meshData.path,
                meshData.smoothNormals,
                meshData.forceNormals);

            auto meshSet = future.get();

            if (meshSet) {
                resolveSockets(
                    meshData,
                    *meshSet
                );

                for (auto& animationData : meshData.animations) {
                    loadAnimation(
                        meshData.baseDir,
                        animationData,
                        *meshSet);
                }

                {
                    KI_INFO_OUT(fmt::format(
                        "\n=======================\n[MESH_SET SUMMARY: {}]\n{}\n=======================",
                        meshSet->m_name,
                        meshSet->getSummary()));
                }

                meshCount += type->addMeshSet(*meshSet);
            }
            break;
        }
        case MeshDataType::primitive: {
            auto mesh = m_loaders->m_vertexLoader.createMesh(meshData, meshData.vertexData, *m_loaders);

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(std::move(mesh), true);
            type->addLodMesh(std::move(lodMesh));

            meshCount++;
            break;
        }
        case MeshDataType::non_vao: {
            auto mesh = std::make_unique<mesh::NonVaoMesh>(type->getName());

            if (meshData.materials.empty()) {
                const auto& material = Material::createMaterial(BasicMaterial::yellow);
                mesh->setMaterial(&material);
            } else {
                const auto* material = &meshData.materials[0].material;
                mesh->setMaterial(material);
            }

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(std::move(mesh), true);
            type->addLodMesh(std::move(lodMesh));

            meshCount++;
            break;
        }
        }

        return meshCount;
    }

    void SceneLoader::resolveLodMesh(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        mesh::LodMesh& lodMesh)
    {
        lodMesh.m_scale = meshData.scale;
        lodMesh.m_baseScale = meshData.baseScale;
        lodMesh.m_baseRotation = util::degreesToQuat(meshData.baseRotation);

        auto* lodData = resolveLod(type, nodeData, meshData, lodMesh);

        assignMeshFlags(meshData.meshFlags, lodMesh);
        if (lodData) {
            assignMeshFlags(lodData->meshFlags, lodMesh);
        }

        resolveMaterials(type, lodMesh, nodeData, meshData, lodData);
        lodMesh.setupDrawOptions();
    }

    const LodData* SceneLoader::resolveLod(
        mesh::MeshType* type,
        const NodeData& nodeData,
        const MeshData& meshData,
        mesh::LodMesh& lodMesh)
    {
        auto* mesh = lodMesh.getMesh<mesh::Mesh>();
        if (!mesh) return nullptr;

        lodMesh.m_priority = nodeData.priority;

        const auto* lod = meshData.findLod(mesh->m_name);
        if (!lod) {
            // NOTE KI if lods defined then default to 0 mask
            if (!meshData.lods.empty()) {
                KI_WARN_OUT(fmt::format("SCENE: MISSING_LOD : mesh={}", mesh->m_name));
                //lodMesh.m_levelMask = 0;
            }
            return nullptr;
        }

        lodMesh.m_minDistance2 = lod->minDistance * lod->minDistance;
        lodMesh.m_maxDistance2 = lod->maxDistance * lod->maxDistance;
        lodMesh.m_priority = lod->priority != 0 ? lod->priority : nodeData.priority;

        return lod;
    }

    void SceneLoader::resolveSockets(
        const MeshData& meshData,
        mesh::MeshSet& meshSet)
    {
        if (!meshSet.m_rig) return;

        auto& rig = *meshSet.m_rig;
        for (const auto& socketData : meshData.sockets) {
            if (!socketData.enabled) continue;

            // TODO KI scale is in LodMesh level, but sockets in Mesh level
            // => PROBLEM if same mesh is used for differently scaled LodMeshes
            //glm::vec3 meshScale{ 0.01375f * 2.f };
            auto meshScale = meshData.scale * meshData.baseScale;

            animation::RigSocket socket{
                socketData.name,
                socketData.joint,
                socketData.offset,
                util::degreesToQuat(socketData.rotation),
                socketData.scale,
                meshScale
            };

            rig.registerSocket(socket);
        }

        rig.prepare();
    }

    void SceneLoader::loadAnimation(
        const std::string& baseDir,
        const AnimationData& data,
        mesh::MeshSet& meshSet)
    {
        if (!meshSet.isRigged()) return;
        if (!*m_ctx.m_alive) return;

        const auto& assets = Assets::get();

        animation::AnimationLoader loader{};

        std::string filePath;
        {
            filePath = util::joinPathExt(
                meshSet.m_rootDir,
                baseDir,
                data.path, "");
        }

        if (!util::fileExists(filePath)) {
            filePath = util::joinPath(
                meshSet.m_rootDir,
                data.path);
        }

        try {
            loader.loadAnimations(
                *meshSet.m_rig,
                data.name,
                filePath);
        }
        catch (animation::AnimationNotFoundError ex) {
            KI_CRITICAL(fmt::format("SCENE_ERROR: LOAD_ANIMATION - {}", ex.what()));
            //throw std::current_exception();
        }
    }

    std::pair<pool::NodeHandle, NodeState> SceneLoader::createNode(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const NodeData& nodeData,
        const bool cloned,
        const int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& clonePositionOffset,
        const glm::vec3& tilePositionOffset)
    {
        auto& l = *m_loaders;
        auto* type = typeHandle.toType();

        ki::node_id nodeId{ 0 };
        std::string resolvedSID;
        {
            if (!nodeData.baseId.empty()) {
                auto [k, v] = resolveId(nodeData.baseId, cloneIndex, tile, false);
                nodeId = k;
                resolvedSID = v;
            }

            if (!nodeId) {
                auto [k, v] = resolveId({ nodeData.name }, cloneIndex, tile, true);
                nodeId = k;
                resolvedSID = v;
            }
        }

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);

        node->setName(resolvedSID);
        node->m_typeHandle = typeHandle;

        {
            ki::node_id ignoredBy{ 0 };
            if (!nodeData.ignoredByBaseId.empty()) {
                auto [id, _] = resolveId(
                    nodeData.ignoredByBaseId,
                    cloneIndex,
                    tile,
                    false);
                ignoredBy = id;
            }
            node->m_ignoredBy = ignoredBy;
        }

        node->m_audioListener = l.m_audioLoader.createListener(nodeData.audio.listener);
        node->m_audioSources = l.m_audioLoader.createSources(nodeData.audio.sources);

        assignNodeFlags(nodeData.nodeFlags, handle);

        //node->setCloneIndex(cloneIndex);
        //node->setTile(tile);

        glm::vec3 pos = nodeData.position + clonePositionOffset + tilePositionOffset;

        NodeState state;
        state.setPosition(pos);

        state.setRotation(util::degreesToQuat(nodeData.rotation));
        state.setScale(nodeData.scale);

        state.setPivot(nodeData.pivot.resolve(type));

        state.setFront(nodeData.front);

        {
            state.setBaseRotation(util::degreesToQuat(nodeData.baseRotation));
            state.setVolume(type->getAABB().getVolume());
        }

        node->m_camera = l.m_cameraLoader.createCamera(nodeData.camera);
        node->m_light = l.m_lightLoader.createLight(nodeData.light, cloneIndex, tile);
        node->m_generator = l.m_generatorLoader.createGenerator(
            nodeData.generator,
            type,
            *m_loaders);

        node->m_particleGenerator = l.m_particleLoader.createParticle(
            nodeData.particle);

        if (type->m_nodeType == NodeType::text) {
            node->m_generator = m_loaders->m_textLoader.createGenerator(
                type,
                nodeData.text,
                *m_loaders);
        }

        resolveAttachments(typeHandle, handle, nodeData);

        return { handle, state };
    }

    void SceneLoader::resolveAttachments(
        pool::TypeHandle typeHandle,
        pool::NodeHandle nodeHandle,
        const NodeData& nodeData)
    {
        if (nodeData.attachments.empty()) return;

        auto* type = typeHandle.toType();
        auto& lodMeshes = type->modifyLodMeshes();

        auto rig = mesh::findRig(lodMeshes);
        if (!rig) {
            KI_INFO_OUT(fmt::format(
                "SOCKET_BIND_ERROR: rig_missing - node={}",
                nodeData.str()));
            return;
        }

        for (const auto& attachment : nodeData.attachments) {
            if (!attachment.enabled) continue;

            mesh::LodMesh* lodMesh = mesh::findLodMesh(attachment.name, lodMeshes);
            if (!lodMesh) {
                const auto& names = mesh::getLodMeshNames(attachment.name, lodMeshes);
                const auto& aliases = mesh::getLodMeshNames(attachment.name, lodMeshes);
                KI_INFO_OUT(fmt::format(
                    "SOCKET_BIND_ERROR: mesh_missing - node={}, rig={}, socket={}, mesh={}, mesh_names=[{}], mesh_aliases=[{}]",
                    nodeData.str(),
                    rig->m_name,
                    attachment.socket,
                    attachment.name,
                    util::join(names, ", "),
                    util::join(aliases, ", ")));
                continue;
            }

            const auto* socket = rig->findSocket(attachment.socket);
            if (!socket) {
                const auto& names = rig->getSocketNames();
                KI_INFO_OUT(fmt::format(
                    "SOCKET_BIND_ERROR: socket_missing - node={}, rig={}, socket={}, mesh={}, socket_names=[{}]",
                    nodeData.str(),
                    rig->m_name,
                    attachment.socket,
                    attachment.name,
                    util::join(names, ", ")));
                continue;
            }

            lodMesh->m_socketIndex = socket->m_index;

            KI_INFO_OUT(fmt::format(
                "SOCKET_BIND_OK: node={}, rig={}, joint={}.{}, socket={}.{}, mesh={}",
                nodeData.str(),
                rig->m_name,
                socket->m_jointIndex,
                socket->m_jointName,
                socket->m_index,
                attachment.socket,
                attachment.name));
        }
    }

    void SceneLoader::assignTypeFlags(
        const NodeData& nodeData,
        pool::TypeHandle typeHandle)
    {
        auto* type = typeHandle.toType();

        const auto& container = nodeData.typeFlags;
        mesh::TypeFlags& flags = type->m_flags;

        //////////////////////////////////////////////////////////
        // LOD_MESH specific
        // Rigged model

        flags.effect = container.getFlag("effect", flags.effect);

        //////////////////////////////////////////////////////////
        // MESH_TYPE specific (aka. Node shared logic)

        flags.mirror = container.getFlag("mirror", flags.mirror);
        flags.water = container.getFlag("water", flags.water);
        flags.cubeMap = container.getFlag("cube_map", flags.cubeMap);

        flags.noFrustum = container.getFlag("no_frustum", flags.noFrustum);
        flags.noShadow = container.getFlag("no_shadow", flags.noShadow);
        flags.noSelect = container.getFlag("no_select", flags.noSelect);
        flags.noReflect = container.getFlag("no_reflect", flags.noReflect);
        flags.noRefract = container.getFlag("no_refract", flags.noRefract);

        {
            flags.staticBounds = container.getFlag("static_bounds", flags.staticBounds);
            flags.dynamicBounds = container.getFlag("dynamic_bounds", flags.dynamicBounds);
            flags.physics = nodeData.physics.enabled;
        }
    }

    void SceneLoader::assignMeshFlags(
        const FlagContainer& container,
        mesh::LodMesh& lodMesh)
    {
        auto& flags = lodMesh.m_flags;

        flags.billboard = container.getFlag("billboard", flags.billboard);
        flags.tessellation = container.getFlag("tessellation", flags.tessellation);

        flags.preDepth = container.getFlag("pre_depth", flags.preDepth);

        flags.noVolume = container.getFlag("no_volume", flags.noVolume);

        {
            flags.useBones = container.getFlag("use_bones", flags.useBones);

            // NOTE KI bones are *required* if using animation
            flags.useAnimation = container.getFlag("use_animation", flags.useAnimation);
            if (flags.useAnimation) {
                flags.useBones = true;
            }

            // NOTE KI no bones debug if no bones
            flags.useBonesDebug = container.getFlag("use_bones_debug", flags.useBonesDebug);
            if (!flags.useBones) {
                flags.useBonesDebug = false;
            }

            flags.useSockets = container.getFlag("use_sockets", flags.useSockets);
        }

        flags.clip = container.getFlag("clip", flags.clip);
    }

    void SceneLoader::assignNodeFlags(
        const FlagContainer& container,
        pool::NodeHandle nodeHandle)
    {
        auto* node = nodeHandle.toNode();

        auto& flags = node->m_flags;
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
        const NodeRoot& nodeRoot,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (nodeRoot.clones.empty()) {
            validateNodeClone(rootId, nodeRoot, nodeRoot.base, false, 0, pass, errorCount, collectedIds);
        }
        else {
            int cloneIndex = 0;
            for (auto& cloneData : nodeRoot.clones) {
                validateNodeClone(rootId, nodeRoot, cloneData, true, cloneIndex, pass, errorCount, collectedIds);
                cloneIndex++;
            }
        }
    }

    void SceneLoader::validateNodeClone(
        const ki::node_id rootId,
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!nodeData.enabled) return;

        const auto& repeat = nodeData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    validateNodeCloneRepeat(
                        rootId,
                        nodeRoot,
                        nodeData,
                        cloned,
                        cloneIndex,
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
        const NodeRoot& nodeRoot,
        const NodeData& nodeData,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!nodeData.enabled) return;

        if (pass == 0) {
            if (nodeData.name == "Linden tree forest - part 1")
                int x = 0;

            ki::node_id sid;
            std::string resolvedSID;
            {
                auto [k, v] = resolveId(
                    nodeData.baseId,
                    cloneIndex, tile,
                    nodeData.baseId.m_path == nodeRoot.base.baseId.m_path);
                sid = k;
                resolvedSID = v;

                if (!sid) {
                    auto [k, v] = resolveId(
                        { nodeData.name },
                        cloneIndex, tile,
                        true);
                    sid = k;
                    resolvedSID = v;
                }
            }

            if (const auto& it = collectedIds.find(sid); it != collectedIds.end()) {
                auto msg = fmt::format("SCENE_ERROR: SID CONFLICT: {} = {} (WAS: {})", sid, resolvedSID, it->second);
                KI_CRITICAL(msg);
                errorCount++;
            }
            collectedIds[sid] = resolvedSID;
        }

        if (pass == 1) {
            // NOTE KI parentId can be *MISSING*
            // but it cannot be duplicate
            if (!nodeData.parentBaseId.empty()) {
                auto [sid, resolvedSID] = resolveId(
                    nodeData.parentBaseId,
                    cloneIndex,
                    tile,
                    false);

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
