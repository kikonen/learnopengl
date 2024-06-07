#include "SceneLoader.h"

#include <string>
#include <vector>
#include <fstream>
#include <string>
#include <span>

#include <fmt/format.h>

#include "util/Util.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"
#include "asset/Material.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MeshSet.h"
#include "mesh/ModelMesh.h"
#include "mesh/QuadMesh.h"
#include "mesh/TextMesh.h"

#include "text/TextDraw.h"

#include "component/Light.h"
#include "component/Camera.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"
#include "model/EntityType.h"

#include "animation/AnimationLoader.h"

//#include "generator/GridGenerator.h"
//#include "generator/AsteroidBeltGenerator.h"
#include "generator/TextGenerator.h"
//#include "terrain/TerrainGenerator.h"

#include "event/Dispatcher.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/ModelRegistry.h"
#include "registry/ProgramRegistry.h"
#include "registry/MeshTypeRegistry.h"

#include <engine/AsyncLoader.h>

#include "DagSort.h"

#include "Loaders.h"
#include "EntityRoot.h"
#include "ResolvedEntity.h"

#include "converter/YamlConverter.h"

#include "loader/document.h"

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
        KI_INFO(fmt::format("SCENE_FILE: delete - ctx={}", m_ctx.str()));
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

                l.m_fontLoader.loadFonts(doc.findNode("fonts"), m_fonts);

                l.m_rootLoader.loadRoot(doc.findNode("root"), *m_root);
                l.m_scriptLoader.loadScriptEngine(doc.findNode("script"), *m_scriptEngineData);

                l.m_entityLoader.loadEntities(
                    doc.findNode("entities"),
                    m_entities,
                    l);

                validate(*m_root);
                attach(*m_root);

                std::lock_guard lock(m_ready_lock);
                m_runningCount--;
            }
            catch (const std::runtime_error& ex) {
                KI_CRITICAL(ex.what());

                std::lock_guard lock(m_ready_lock);
                m_runningCount--;
            }
        });
    }

    void SceneLoader::loadedEntity(
        const EntityRoot& entityRoot,
        bool success)
    {
        std::lock_guard lock(m_ready_lock);

        m_pendingCount--;

        KI_INFO_OUT(fmt::format(
            "LOADED: entity={}, success={}, pending={}",
            entityRoot.base.name, success, m_pendingCount));

        if (m_pendingCount > 0) return;

        // NOTE KI event will be put queue *AFTER* entity attach events
        // => should they should be fully attached in scene at this point
        // => worker will trigger event into UI thread after processing all updates

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this]() {
            try {
                attachResolvedEntities(m_resolvedEntities);
                notifySceneLoaded();
            }
            catch (const std::runtime_error& ex) {
                throw ex;
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
        l.m_scriptLoader.createScriptEngine(root.rootId, *m_scriptEngineData);

        l.m_skyboxLoader.attachSkybox(root.rootId, *m_skybox);
        l.m_volumeLoader.attachVolume(root.rootId);
        l.m_cubeMapLoader.attachCubeMap(root.rootId);

        l.m_fontLoader.createFonts(m_fonts);

        {
            std::lock_guard lock(m_ready_lock);

            m_pendingCount = 0;
            for (const auto& entity : m_entities) {
                if (resolveEntity(root.rootId, entity)) {
                    m_pendingCount++;
                    KI_INFO_OUT(fmt::format("START: entity={}, pending={}", entity.base.name, m_pendingCount));
                }
            }

            KI_INFO_OUT(fmt::format("TOTAL: pending={}", m_pendingCount));
        }
    }

    void SceneLoader::attachResolvedEntities(
        std::vector<ResolvedEntity>& resolvedEntities)
    {
        auto& l = *m_loaders;

        DagSort sorter;
        auto sorted = sorter.sort(resolvedEntities);

        for (auto* resolved : sorted) {
            if (!*m_ctx.m_alive) return;
            attachResolvedEntity(*resolved);
        }
    }

    void SceneLoader::attachResolvedEntity(
        const ResolvedEntity& resolved)
    {
        auto& l = *m_loaders;
        auto& handle = resolved.handle;
        auto& entityData = resolved.data;

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = handle.toId(),
                .parentId = resolved.parentId,
            };
            m_dispatcher->send(evt);
        }

        if (entityData.active) {
            event::Event evt { event::Type::node_activate };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        if (entityData.selected) {
            event::Event evt { event::Type::node_select };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        if (entityData.camera.isDefault) {
            event::Event evt { event::Type::camera_activate };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        for (auto& controllerData : entityData.controllers) {
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
        //if (!entity.isRoot && !type->m_flags.water && !type->m_flags.tessellation && !type->m_flags.noShadow)
        //if (entityData.desc == "Cow")
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

        {
            l.m_audioLoader.createAudio(entityData.audio, handle.toId());
            l.m_physicsLoader.createObject(entityData.physics, handle.toId());
        }
    }

    void SceneLoader::addResolvedEntity(
        const ResolvedEntity& resolved)
    {
        {
            std::lock_guard lock(m_ready_lock);
            m_resolvedEntities.push_back(resolved);
        }
    }

    bool SceneLoader::resolveEntity(
        const ki::node_id rootId,
        const EntityRoot& entityRoot)
    {
        if (!entityRoot.base.enabled) {
            return false;
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this, rootId, &entityRoot]() {
            try {
                if (entityRoot.clones.empty()) {
                    pool::TypeHandle typeHandle{};
                    resolveEntityClone(typeHandle, rootId, entityRoot, entityRoot.base, false, 0);
                }
                else {
                    pool::TypeHandle typeHandle{};

                    int cloneIndex = 0;
                    for (auto& cloneData : entityRoot.clones) {
                        if (!*m_ctx.m_alive) return;
                        typeHandle = resolveEntityClone(typeHandle, rootId, entityRoot, cloneData, true, cloneIndex);
                        if (!entityRoot.base.cloneMesh) {
                            typeHandle = pool::TypeHandle::NULL_HANDLE;
                        }
                        cloneIndex++;
                    }
                }
                loadedEntity(entityRoot, true);
            }
            catch (const std::runtime_error& ex) {
                loadedEntity(entityRoot, false);
                throw ex;
            }
        });

        return true;
    }

    pool::TypeHandle SceneLoader::resolveEntityClone(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const EntityRoot& entityRoot,
        const EntityData& entityData,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx.m_alive) return typeHandle;

        if (!entityData.enabled) {
            return typeHandle;
        }

        const auto& repeat = entityData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    if (!*m_ctx.m_alive) return typeHandle;

                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    typeHandle = resolveEntityCloneRepeat(
                        typeHandle,
                        rootId,
                        entityRoot,
                        entityData,
                        cloned,
                        cloneIndex,
                        tile,
                        tilePositionOffset);

                    if (!entityRoot.base.cloneMesh)
                        typeHandle = pool::TypeHandle::NULL_HANDLE;
                }
            }
        }

        return typeHandle;
    }

    pool::TypeHandle SceneLoader::resolveEntityCloneRepeat(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const EntityRoot& entityRoot,
        const EntityData& entityData,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset)
    {
        if (!*m_ctx.m_alive) return typeHandle;

        if (!entityData.enabled) {
            return typeHandle;
        }

        // NOTE KI overriding material in clones is *NOT* supported"
        if (!typeHandle) {
            typeHandle = createType(
                entityData,
                tile);
            if (!typeHandle) return typeHandle;
        }

        if (!*m_ctx.m_alive) return typeHandle;

        auto handle = createNode(
            typeHandle, rootId, entityData,
            cloned, cloneIndex, tile,
            entityData.clonePositionOffset,
            tilePositionOffset);

        ki::node_id parentId;
        if (entityData.parentBaseId.empty()) {
            parentId = rootId;
        }
        else {
            auto [id, _] = resolveId(
                entityData.parentBaseId,
                cloneIndex,
                tile,
                false);
            parentId = id;
        }

        ResolvedEntity resolved{
            parentId,
            handle,
            entityData
        };

        addResolvedEntity(resolved);

        return typeHandle;
    }

    const pool::TypeHandle SceneLoader::createType(
        const EntityData& entityData,
        const glm::uvec3& tile)
    {
        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName(entityData.baseId.m_path);

        assignFlags(entityData, typeHandle);

        type->m_priority = entityData.priority;

        if (entityData.type == EntityType::origo) {
            type->m_flags.invisible = true;
            type->m_entityType = EntityType::origo;
        } else
        {
            resolveMeshes(type, entityData, tile);
            resolveLods(type, entityData);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (entityData.type != EntityType::container) {
                if (!type->hasMesh()) {
                    KI_WARN(fmt::format(
                        "SCENE_FILEIGNORE: NO_MESH id={} ({})",
                        entityData.baseId, entityData.desc));
                    return pool::TypeHandle::NULL_HANDLE;
                }
            }

            // NOTE KI DEP: program after materials
            resolveProgram(typeHandle, entityData);
        }

        return typeHandle;
    }

    void SceneLoader::resolveProgram(
        pool::TypeHandle typeHandle,
        const EntityData& entityData)
    {
        auto* type = typeHandle.toType();

        bool useTBN = false;
        bool useParallax = false;
        bool useDudvTex = false;
        bool useDisplacementTex = false;
        bool useNormalTex = false;
        bool useCubeMap = false;
        bool useNormalPattern = false;

        for (auto& lodMesh : type->modifyLodMeshes()) {
            auto* lodMaterial = lodMesh.getMaterial();

            if (lodMaterial) {
                auto& material = *lodMaterial;

                useDudvTex |= material.hasTex(MATERIAL_DUDV_MAP_IDX);
                useDisplacementTex |= material.hasTex(MATERIAL_DISPLACEMENT_MAP_IDX);
                useNormalTex |= material.hasTex(MATERIAL_NORMAL_MAP_IDX);
                useCubeMap |= 1.0 - material.reflection - material.refraction < 1.0;
                useNormalPattern |= material.pattern > 0;
                useParallax |= material.hasTex(MATERIAL_DISPLACEMENT_MAP_IDX) && material.parallaxDepth > 0;
                if (!useParallax) {
                    material.parallaxDepth = 0.f;
                }
            }
        }

        useTBN = useNormalTex || useDudvTex || useDisplacementTex;

        if (!entityData.programName.empty()) {
            std::map<std::string, std::string, std::less<>> definitions;
            std::map<std::string, std::string, std::less<>> shadowDefinitions;
            std::map<std::string, std::string, std::less<>> selectionDefinitions;
            std::map<std::string, std::string, std::less<>> idDefinitions;

            for (const auto& [k, v] : entityData.programDefinitions) {
                definitions[k] = v;
            }

            std::map<std::string, std::string, std::less<>> preDepthDefinitions;
            bool usePreDepth = type->m_flags.preDepth;
            bool useBones = type->m_flags.useBones;
            bool useBonesDebug = useBones && type->m_flags.useBonesDebug;

            if (type->m_flags.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
                shadowDefinitions[DEF_USE_ALPHA] = "1";
                selectionDefinitions[DEF_USE_ALPHA] = "1";
                idDefinitions[DEF_USE_ALPHA] = "1";
                usePreDepth = false;
            }
            if (type->m_flags.blend) {
                definitions[DEF_USE_BLEND] = "1";
                usePreDepth = false;
            }
            if (type->m_flags.blendOIT) {
                definitions[DEF_USE_BLEND_OIT] = "1";
                usePreDepth = false;
            }

            //if (type->m_entityType == EntityType::billboard) {
            //    definitions[DEF_USE_BILLBOARD] = "1";
            //}

            if (useTBN) {
                definitions[DEF_USE_TBN] = "1";
            }
            //if (useDudvTex) {
            //    definitions[DEF_USE_DUDV_TEX] = "1";
            //}
            //if (useDisplacementTex) {
            //    definitions[DEF_USE_DISPLACEMENT_TEX] = "1";
            //}
            if (useNormalTex) {
                definitions[DEF_USE_NORMAL_TEX] = "1";
            }
            if (useParallax) {
                definitions[DEF_USE_PARALLAX] = "1";
            }
            if (useCubeMap) {
                definitions[DEF_USE_CUBE_MAP] = "1";
            }
            if (useNormalPattern) {
                definitions[DEF_USE_NORMAL_PATTERN] = "1";
            }
            if (useBones) {
                definitions[DEF_USE_BONES] = "1";
                shadowDefinitions[DEF_USE_BONES] = "1";
                selectionDefinitions[DEF_USE_BONES] = "1";
                idDefinitions[DEF_USE_BONES] = "1";
            }
            if (useBonesDebug) {
                definitions[DEF_USE_BONES_DEBUG] = "1";
            }

            type->m_program = ProgramRegistry::get().getProgram(
                entityData.programName,
                false,
                entityData.geometryType,
                definitions);

            if (!entityData.shadowProgramName.empty()) {
                type->m_shadowProgram = ProgramRegistry::get().getProgram(
                    entityData.shadowProgramName,
                    false,
                    "",
                    shadowDefinitions);
            }

            if (usePreDepth) {
                type->m_preDepthProgram = ProgramRegistry::get().getProgram(
                    entityData.preDepthProgramName,
                    false,
                    "",
                    preDepthDefinitions);
            }

            if (!entityData.selectionProgramName.empty()) {
                type->m_selectionProgram = ProgramRegistry::get().getProgram(
                    entityData.selectionProgramName,
                    false,
                    "",
                    selectionDefinitions);
            }

            if (!entityData.idProgramName.empty()) {
                type->m_idProgram = ProgramRegistry::get().getProgram(
                    entityData.idProgramName,
                    false,
                    "",
                    idDefinitions);
            }
        }
    }

    text::font_id SceneLoader::resolveFont(
        pool::TypeHandle typeHandle,
        const TextData& data) const
     {
        auto* font = findFont(data.font);
        return font ? font->id : 0;
    }

    void SceneLoader::resolveMaterials(
        mesh::MeshType* type,
        mesh::LodMesh& lodMesh,
        const MeshData& meshData)
    {
        auto* lodMaterial = lodMesh.getMaterial();
        if (!lodMaterial) return;

        auto& material = *lodMaterial;
        auto& l = *m_loaders;

        for (auto& materialData : meshData.materials) {
            const auto& alias = materialData.aliasName;
            const auto& name = materialData.materialName;
            KI_INFO_OUT(fmt::format("MAT_REF: model={}, name={}, alias={}", type->str(), name, alias));
            if (alias == material.m_name || name == material.m_name || alias.empty() || alias == "*")
            {
                //if (!name.empty() && !alias.empty()) {
                if (alias == material.m_name || name == material.m_name || alias == "*") {
                    material.assign(materialData.material);
                }
                l.m_materialLoader.modifyMaterial(material, materialData);
            }
        }
        material.loadTextures();
    }

    void SceneLoader::resolveMeshes(
        mesh::MeshType* type,
        const EntityData& entityData,
        const glm::uvec3& tile)
    {
        uint16_t index = 0;
        for (const auto& meshData : entityData.meshes) {
            resolveMesh(type, entityData, meshData, tile, index);
            index++;
        }
    }

    void SceneLoader::resolveMesh(
        mesh::MeshType* type,
        const EntityData& entityData,
        const MeshData& meshData,
        const glm::uvec3& tile,
        int index)
    {
        const auto& assets = Assets::get();

        size_t startIndex = type->m_lodMeshes->size();
        size_t meshCount = 0;

        // NOTE KI materials MUST be resolved before loading mesh
        if (entityData.type == EntityType::model) {
            type->m_entityType = EntityType::model;

            auto future = ModelRegistry::get().getMeshSet(
                assets.modelsDir,
                meshData.meshPath);

            auto meshSet = future.get();

            if (meshSet) {
                for (auto& animationData : meshData.animations) {
                    loadAnimation(
                        meshData.baseDir,
                        animationData,
                        *meshSet);
                }

                meshCount += type->addMeshSet(
                    *meshSet,
                    meshData.level);
            }

            KI_INFO(fmt::format(
                "SCENE_FILE MESH: id={}, desc={}, type={}",
                entityData.baseId, entityData.desc, type->str()));
        }
        else if (entityData.type == EntityType::text) {
            type->m_entityType = EntityType::text;
            auto mesh = std::make_unique<mesh::TextMesh>();

            mesh::LodMesh lodMesh;
            lodMesh.setMesh(std::move(mesh), true);
            type->addLodMesh(std::move(lodMesh));
            meshCount++;
        }
        else if (entityData.type == EntityType::terrain) {
            // NOTE KI handled via container + generator
            type->m_entityType = EntityType::terrain;
            throw "Terrain not supported currently";
        }
        else if (entityData.type == EntityType::container) {
            // NOTE KI generator takes care of actual work
            type->m_entityType = EntityType::container;
            type->m_flags.invisible = true;
        }
        else {
            // NOTE KI root/origo/unknown; don't render, just keep it in hierarchy
            type->m_entityType = EntityType::origo;
            type->m_flags.invisible = true;
        }

        // Resolve materials for newly added meshes
        if (meshCount > 0) {
            const auto& span = std::span{ *type->m_lodMeshes.get() }.subspan(startIndex, meshCount);
            for (auto& lodMesh : span) {
                resolveMaterials(type, lodMesh, meshData);
            }
        }
    }

    void SceneLoader::resolveLods(
        mesh::MeshType* type,
        const EntityData& entityData)
    {
        for (const auto& lodData : entityData.lods) {
            resolveLod(type, lodData);
        }
    }

    void SceneLoader::resolveLod(
        mesh::MeshType* type,
        const LodData& lodData)
    {
        for (auto& lodMesh : *type->m_lodMeshes) {
            if (lodMesh.m_lodLevel == lodData.level) {
                lodMesh.setDistance(lodData.distance);
            }
        }
    }

    void SceneLoader::loadAnimation(
        const std::string& baseDir,
        const AnimationData& data,
        mesh::MeshSet& meshSet)
    {
        if (!meshSet.isRigged()) return;

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

        loader.loadAnimations(
            *meshSet.m_rig,
            data.name,
            filePath);
    }

    pool::NodeHandle SceneLoader::createNode(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const EntityData& entityData,
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
            if (!entityData.baseId.empty()) {
                auto [k, v] = resolveId(entityData.baseId, cloneIndex, tile, false);
                nodeId = k;
                resolvedSID = v;
            }

            if (!nodeId) {
                auto [k, v] = resolveId({ entityData.name }, cloneIndex, tile, true);
                nodeId = k;
                resolvedSID = v;
            }
        }

        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
        assert(node);
#ifdef _DEBUG
        node->m_resolvedSID = resolvedSID;
#endif
        node->m_typeHandle = typeHandle;

        node->setCloneIndex(cloneIndex);
        //node->setTile(tile);

        glm::vec3 pos = entityData.position + clonePositionOffset + tilePositionOffset;

        auto& transform = node->modifyTransform();
        transform.setPosition(pos);

        transform.setQuatRotation(util::degreesToQuat(entityData.rotation));
        transform.setBaseScale(entityData.baseScale);
        transform.setScale(entityData.scale);
        transform.setFront(entityData.front);

        {
            auto baseTransform = glm::toMat4(util::degreesToQuat(entityData.baseRotation));
            transform.setVolume(type->getAABB().getVolume());
            transform.setBaseTransform(baseTransform);
        }

        node->m_camera = l.m_cameraLoader.createCamera(entityData.camera);
        node->m_light = l.m_lightLoader.createLight(entityData.light, cloneIndex, tile);
        node->m_generator = l.m_generatorLoader.createGenerator(
            entityData.generator,
            type);

        node->m_particleGenerator = l.m_particleLoader.createParticle(
            entityData.particle);

        if (type->m_entityType == EntityType::text) {
            auto fontId = resolveFont(typeHandle, entityData.text);
            auto generator = std::make_unique<TextGenerator>();
            generator->setFontId(fontId);
            generator->setText(entityData.text.text);
            node->m_generator = std::move(generator);
        }

        l.m_scriptLoader.createScript(
            rootId,
            node->getId(),
            entityData.script);

        {
            type->setCustomMaterial(
                l.m_customMaterialLoader.createCustomMaterial(
                    entityData.customMaterial,
                    cloneIndex,
                    tile));
            MeshTypeRegistry::get().registerCustomMaterial(typeHandle);
        }

        return handle;
    }

    void SceneLoader::assignFlags(
        const EntityData& entityData,
        pool::TypeHandle typeHandle)
    {
        auto* type = typeHandle.toType();

        const auto& container = entityData.typeFlags;
        mesh::TypeFlags& flags = type->m_flags;

        flags.gbuffer = entityData.programName.starts_with("g_");

        // Rigged model
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
        }

        flags.preDepth = container.getFlag("pre_depth", flags.preDepth);
        flags.gbuffer = container.getFlag("gbuffer", flags.gbuffer);
        flags.alpha = container.getFlag("alpha", flags.alpha);

        {
            const auto* e = container.getOptional("blend");
            if (e) {
                flags.blend = e;
                // NOTE KI alpha MUST BE true if blend
                if (flags.blend) {
                    KI_WARN(fmt::format("BLEND requires alpha (enabled alpha): id={}, desc={}", entityData.baseId, entityData.desc));
                    flags.alpha = true;
                }
                // NOTE KI blend CANNOT be gbuffer
                if (flags.blend && flags.gbuffer) {
                    KI_ERROR(fmt::format("GBUFFER vs. BLEND mismatch (disabled blend): id={}, desc={}", entityData.baseId, entityData.desc));
                    // NOTE KI turning off blend; shader is designed for gbuffer
                    flags.blend = false;
                }
            }
        }
        {
            const auto* e = container.getOptional("blend_oit");
            if (e) {
                flags.blendOIT = e;
                // NOTE KI alpha MUST BE true if blend
                if (flags.blendOIT) {
                    KI_WARN(fmt::format("BLEND requires alpha (enabled alpha): id={}, desc={}", entityData.baseId, entityData.desc));
                    flags.alpha = true;
                }
            }
        }

        flags.zUp = container.getFlag("z_up", flags.renderBack);
        flags.renderBack = container.getFlag("render_back", flags.renderBack);
        flags.noFrustum = container.getFlag("no_frustum", flags.noFrustum);
        flags.noShadow = container.getFlag("no_shadow", flags.noShadow);
        flags.noSelect = container.getFlag("no_select", flags.noSelect);
        flags.noReflect = container.getFlag("no_reflect", flags.noReflect);
        flags.noRefract = container.getFlag("no_refract", flags.noRefract);
        flags.mirror = container.getFlag("mirror", flags.mirror);
        flags.water = container.getFlag("water", flags.water);
        flags.wireframe = container.getFlag("wireframe", flags.wireframe);
        flags.cubeMap = container.getFlag("cube_map", flags.cubeMap);
        flags.effect = container.getFlag("effect", flags.effect);
        flags.billboard = container.getFlag("billboard", flags.billboard);
        flags.tessellation = container.getFlag("tessellation", flags.tessellation);

        {
            const auto* e = container.getOptional("static_bounds");
            if (e) {
                flags.staticBounds = e;
                flags.physics = e;
            }
        }
        {
            const auto* e = container.getOptional("dynamic_bounds");
            if (e) {
                flags.dynamicBounds = e;
                flags.physics = e;
            }
        }

        if (entityData.physics.enabled || flags.staticBounds || flags.dynamicBounds) {
            flags.physics = true;
        }
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

        for (const auto& entity : m_entities) {
            validateEntity(root.rootId, entity, 0, pass1Errors, collectedIds);
        }

        for (const auto& entity : m_entities) {
            validateEntity(root.rootId, entity, 1, pass2Errors, collectedIds);
        }

        if (pass1Errors > 0 || pass2Errors > 0) {
            auto msg = fmt::format("VALIDATE: FAILED - pass1={}, pass2={}", pass1Errors, pass2Errors);
            KI_CRITICAL(msg);
            throw std::runtime_error{ msg };
        }
    }

    void SceneLoader::validateEntity(
        const ki::node_id rootId,
        const EntityRoot& entityRoot,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (entityRoot.clones.empty()) {
            validateEntityClone(rootId, entityRoot, entityRoot.base, false, 0, pass, errorCount, collectedIds);
        }
        else {
            int cloneIndex = 0;
            for (auto& cloneData : entityRoot.clones) {
                validateEntityClone(rootId, entityRoot, cloneData, true, cloneIndex, pass, errorCount, collectedIds);
                cloneIndex++;
            }
        }
    }

    void SceneLoader::validateEntityClone(
        const ki::node_id rootId,
        const EntityRoot& entityRoot,
        const EntityData& entityData,
        bool cloned,
        int cloneIndex,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!entityData.enabled) return;

        const auto& repeat = entityData.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    validateEntityCloneRepeat(
                        rootId,
                        entityRoot,
                        entityData,
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

    void SceneLoader::validateEntityCloneRepeat(
        const ki::node_id rootId,
        const EntityRoot& entityRoot,
        const EntityData& entityData,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!entityData.enabled) return;

        if (pass == 0) {
            if (entityData.name == "Linden tree forest - part 1")
                int x = 0;

            ki::node_id sid;
            std::string resolvedSID;
            {
                auto [k, v] = resolveId(
                    entityData.baseId,
                    cloneIndex, tile,
                    entityData.baseId.m_path == entityRoot.base.baseId.m_path);
                sid = k;
                resolvedSID = v;

                if (!sid) {
                    auto [k, v] = resolveId(
                        { entityData.name },
                        cloneIndex, tile,
                        true);
                    sid = k;
                    resolvedSID = v;
                }
            }

            if (const auto& it = collectedIds.find(sid); it != collectedIds.end()) {
                auto msg = fmt::format("SID CONFLICT: {} = {} (WAS: {})", sid, resolvedSID, it->second);
                KI_CRITICAL(msg);
                errorCount++;
            }
            collectedIds[sid] = resolvedSID;
        }

        if (pass == 1) {
            // NOTE KI parentId can be *MISSING*
            // but it cannot be duplicate
            if (!entityData.parentBaseId.empty()) {
                auto [sid, resolvedSID] = resolveId(
                    entityData.parentBaseId,
                    cloneIndex,
                    tile,
                    false);

                // TODO KI validate missing
                if (collectedIds.find(sid) == collectedIds.end()) {
                    auto msg = fmt::format("PARENT SID MISSING: {} = {}", sid, resolvedSID);
                    KI_CRITICAL(msg);
                    errorCount++;
                }
            }
        }
    }

    const FontData* SceneLoader::findFont(
        std::string_view name) const
    {
        const auto& it = std::find_if(
            m_fonts.cbegin(),
            m_fonts.cend(),
            [&name](const auto& m) { return m.name == name; });
        return it != m_fonts.end() ? &(*it) : nullptr;
    }

}
