#include "SceneLoader.h"

#include <string>
#include <vector>
#include <fstream>
#include <string>

#include <fmt/format.h>

#include "util/Util.h"
#include "util/glm_format.h"
#include "util/glm_util.h"

#include "ki/yaml.h"
#include "ki/sid.h"

#include "pool/NodeHandle.h"
#include "pool/TypeHandle.h"

#include "asset/Assets.h"
#include "asset/Material.h"
#include "asset/Sprite.h"
#include "asset/Shape.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "mesh/EntityType.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/ModelMesh.h"
#include "mesh/QuadMesh.h"
#include "mesh/SpriteMesh.h"
#include "mesh/TextMesh.h"

#include "text/TextDraw.h"

#include "component/Light.h"
#include "component/Camera.h"

#include "particle/ParticleGenerator.h"

#include "model/Node.h"

#include "animation/AnimationLoader.h"
#include "animation/RigContainer.h"

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
                std::ifstream fin(this->m_ctx.m_fullPath);
                YAML::Node doc = YAML::Load(fin);

                loadMeta(doc["meta"], *m_meta);

                auto& l = *m_loaders;

                l.m_skyboxLoader.loadSkybox(doc["skybox"], *m_skybox);

                l.m_fontLoader.loadFonts(doc["fonts"], m_fonts);
                l.m_materialLoader.loadMaterials(doc["materials"], m_materials);
                l.m_spriteLoader.loadSprites(doc["sprites"], m_sprites);

                l.m_rootLoader.loadRoot(doc["root"], *m_root);
                l.m_scriptLoader.loadScriptEngine(doc["script"], *m_scriptEngineData);

                l.m_entityLoader.loadEntities(
                    doc["entities"],
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
        auto& data = resolved.data;

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = handle.toId(),
                .parentId = resolved.parentId,
            };
            m_dispatcher->send(evt);
        }

        if (data.active) {
            event::Event evt { event::Type::node_activate };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        if (data.selected) {
            event::Event evt { event::Type::node_select };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        if (data.camera.isDefault) {
            event::Event evt { event::Type::camera_activate };
            evt.body.node = {
                .target = handle.toId(),
            };
            m_dispatcher->send(evt);
        }

        for (auto& controllerData : data.controllers) {
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
        if (data.desc == "Cow")
        {
            event::Event evt { event::Type::animate_rotate };
            evt.body.animate = {
                .target = handle.toId(),
                .duration = 20,
                .data = { 0, 1.f, 0 },
                .data2 = { 360.f, 0.f, 0.f },
            };
            m_dispatcher->send(evt);
        }

        {
            l.m_audioLoader.createAudio(data.audio, handle.toId());
            l.m_physicsLoader.createObject(data.physics, handle.toId());
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
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx.m_alive) return typeHandle;

        if (!data.enabled) {
            return typeHandle;
        }

        const auto& repeat = data.repeat;

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
                        data,
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
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset)
    {
        if (!*m_ctx.m_alive) return typeHandle;

        if (!data.enabled) {
            return typeHandle;
        }

        // NOTE KI overriding material in clones is *NOT* supported"
        if (!typeHandle) {
            typeHandle = createType(
                data,
                tile);
            if (!typeHandle) return typeHandle;
        }

        if (!*m_ctx.m_alive) return typeHandle;

        auto handle = createNode(
            typeHandle, rootId, data,
            cloned, cloneIndex, tile,
            data.clonePositionOffset,
            tilePositionOffset);

        ki::node_id parentId;
        if (data.parentBaseId.empty()) {
            parentId = rootId;
        }
        else {
            auto [id, _] = resolveId(
                data.parentBaseId,
                cloneIndex,
                tile,
                false);
            parentId = id;
        }

        ResolvedEntity resolved{
            parentId,
            handle,
            data
        };

        addResolvedEntity(resolved);

        return typeHandle;
    }

    const pool::TypeHandle SceneLoader::createType(
        const EntityCloneData& data,
        const glm::uvec3& tile)
    {
        auto typeHandle = pool::TypeHandle::allocate();
        auto* type = typeHandle.toType();
        type->setName(data.baseId.m_path);

        assignFlags(data, typeHandle);

        type->m_priority = data.priority;

        if (data.type == mesh::EntityType::origo) {
            type->m_flags.invisible = true;
            type->m_entityType = mesh::EntityType::origo;
        } else
        {
            resolveSprite(typeHandle, data);
            resolveMesh(typeHandle, data, tile);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (data.type != mesh::EntityType::container) {
                if (!type->hasMesh()) {
                    KI_WARN(fmt::format(
                        "SCENE_FILEIGNORE: NO_MESH id={} ({})",
                        data.baseId, data.desc));
                    return pool::TypeHandle::NULL_HANDLE;
                }
            }

            int lodIndex = 0;
            for (auto& lodData : data.lods) {
                resolveMaterials(typeHandle, data, lodData, lodIndex++);
            }

            // NOTE KI DEP: program after materials
            resolveProgram(typeHandle, data);
        }

        return typeHandle;
    }

    void SceneLoader::resolveProgram(
        pool::TypeHandle typeHandle,
        const EntityCloneData& data)
    {
        auto* type = typeHandle.toType();

        bool useTBN = false;
        bool useParallax = false;
        bool useDudvTex = false;
        bool useDisplacementTex = false;
        bool useNormalTex = false;
        bool useCubeMap = false;
        bool useNormalPattern = false;

        for (auto& lod : type->modifyLods()) {
            for (auto& m : lod.m_materialSet.modifyMaterials()) {
                useDudvTex |= m.hasTex(MATERIAL_DUDV_MAP_IDX);
                useDisplacementTex |= m.hasTex(MATERIAL_DISPLACEMENT_MAP_IDX);
                useNormalTex |= m.hasTex(MATERIAL_NORMAL_MAP_IDX);
                useCubeMap |= 1.0 - m.reflection - m.refraction < 1.0;
                useNormalPattern |= m.pattern > 0;
                useParallax |= m.hasTex(MATERIAL_DISPLACEMENT_MAP_IDX) && m.parallaxDepth > 0;
                if (!useParallax) {
                    m.parallaxDepth = 0.f;
                }
            }
        }

        useTBN = useNormalTex || useDudvTex || useDisplacementTex;

        if (!data.programName.empty()) {
            std::map<std::string, std::string, std::less<>> definitions;
            std::map<std::string, std::string, std::less<>> shadowDefinitions;

            for (const auto& [k, v] : data.programDefinitions) {
                definitions[k] = v;
            }

            std::map<std::string, std::string, std::less<>> preDepthDefinitions;
            bool usePreDepth = type->m_flags.preDepth;
            bool useBones = type->m_flags.useBones;
            bool useBonesDebug = useBones && type->m_flags.useBonesDebug;

            if (type->m_flags.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
                shadowDefinitions[DEF_USE_ALPHA] = "1";
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
            //if (type->m_entityType == EntityType::sprite) {
            //    definitions[DEF_USE_SPRITE] = "1";
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
            }
            if (useBonesDebug) {
                definitions[DEF_USE_BONES_DEBUG] = "1";
            }

            type->m_program = ProgramRegistry::get().getProgram(
                data.programName,
                false,
                data.geometryType,
                definitions);

            if (!data.shadowProgramName.empty()) {
                type->m_shadowProgram = ProgramRegistry::get().getProgram(
                    data.shadowProgramName,
                    false,
                    "",
                    shadowDefinitions);
            }

            if (usePreDepth) {
                type->m_preDepthProgram = ProgramRegistry::get().getProgram(
                    data.preDepthProgramName,
                    false,
                    "",
                    preDepthDefinitions);
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
        pool::TypeHandle typeHandle,
        const EntityCloneData& entityData,
        const LodData& data,
        int lodIndex)
    {
        auto& l = *m_loaders;
        auto* type = typeHandle.toType();

        auto* lodMesh = type->modifyLod(lodIndex);
        if (lodMesh) {
            lodMesh->setupMeshMaterials(*m_defaultMaterial, true, false);

            for (auto& m : lodMesh->m_materialSet.modifyMaterials()) {
                for (auto& ref : data.materialReferences) {
                    const auto& alias = ref.modifiers.aliasName;
                    const auto& name = ref.modifiers.materialName;
                    KI_INFO_OUT(fmt::format("MAT_REF: model={}, name={}, alias={}", type->str(), name, alias));
                    if (alias == m.m_name || alias.empty() || alias == "*")
                    {
                        if (!name.empty() && !alias.empty()) {
                            const auto* overrideMaterial = findMaterial(name, m_materials);
                            if (overrideMaterial) {
                                m.assign(*overrideMaterial);
                            }
                        }
                        l.m_materialLoader.modifyMaterial(m, ref.modifiers);
                    }
                }
                m.loadTextures();
            };
        }
    }

    void SceneLoader::resolveSprite(
        pool::TypeHandle typeHandle,
        const EntityCloneData& data)
    {
        auto* type = typeHandle.toType();

        const Sprite* sprite{ nullptr };

        if (!data.spriteName.empty()) {
            sprite = findSprite(data.spriteName);
        }

        if (sprite) {
            type->m_sprite = std::make_unique<Sprite>(*sprite);
        }
    }

    void SceneLoader::resolveMesh(
        pool::TypeHandle typeHandle,
        const EntityCloneData& data,
        const glm::uvec3& tile)
    {
        const auto& assets = Assets::get();

        auto* type = typeHandle.toType();

        // NOTE KI materials MUST be resolved before loading mesh
        if (data.type == mesh::EntityType::model) {
            type->m_entityType = mesh::EntityType::model;

            for (auto& lodData : data.lods) {
                auto future = ModelRegistry::get().getMesh(
                    assets.modelsDir,
                    lodData.meshPath);

                auto* lod = type->addLod({ future.get() });
                lod->m_lod.setDistance(lodData.distance);

                KI_INFO(fmt::format(
                    "SCENE_FILE ATTACH: id={}, desc={}, type={}",
                    data.baseId, data.desc, type->str()));
            }

            for (auto& animationData : data.animations) {
                resolveAnimation(typeHandle, animationData);
            }
        }
        else if (data.type == mesh::EntityType::sprite) {
            auto future = ModelRegistry::get().getMesh(
                assets.modelsDir,
                QUAD_MESH_NAME);
            type->addLod({ future.get() });
            type->m_entityType = mesh::EntityType::sprite;
        }
        else if (data.type == mesh::EntityType::text) {
            type->m_entityType = mesh::EntityType::text;
            auto mesh = std::make_unique<mesh::TextMesh>();

            mesh::LodMesh lod;
            lod.setMesh(std::move(mesh), true);
            type->addLod(std::move(lod));
        }
        else if (data.type == mesh::EntityType::terrain) {
            type->m_entityType = mesh::EntityType::terrain;
        }
        else if (data.type == mesh::EntityType::container) {
            // NOTE KI generator takes care of actual work
            type->m_entityType = mesh::EntityType::container;
            type->m_flags.invisible = true;
        }
        else {
            // NOTE KI root/origo/unknown; don't render, just keep it in hierarchy
            type->m_entityType = mesh::EntityType::origo;
            type->m_flags.invisible = true;
        }
    }

    void SceneLoader::resolveAnimation(
        pool::TypeHandle typeHandle,
        const AnimationData& data)
    {
        const auto& assets = Assets::get();

        auto* type = typeHandle.toType();

        auto* mesh = type->modifyLod(0)->getMesh<mesh::ModelMesh>();

        if (!mesh->m_rig) return;
        if (!mesh->m_rig->hasBones()) return;

        animation::AnimationLoader loader{};

        std::string filePath = util::joinPath(
            mesh->m_rootDir,
            data.path);

        loader.loadAnimations(
            *mesh->m_rig,
            data.name,
            filePath);
    }

    pool::NodeHandle SceneLoader::createNode(
        pool::TypeHandle typeHandle,
        const ki::node_id rootId,
        const EntityCloneData& data,
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
            if (!data.baseId.empty()) {
                auto [k, v] = resolveId(data.baseId, cloneIndex, tile, false);
                nodeId = k;
                resolvedSID = v;
            }

            if (!nodeId) {
                auto [k, v] = resolveId({ data.name }, cloneIndex, tile, true);
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

        glm::vec3 pos = data.position + clonePositionOffset + tilePositionOffset;

        auto& transform = node->modifyTransform();
        transform.setPosition(pos);

        transform.setQuatRotation(util::degreesToQuat(data.rotation));
        transform.setBaseScale(data.baseScale);
        transform.setScale(data.scale);
        transform.setFront(data.front);

        auto baseTransform = glm::toMat4(util::degreesToQuat(data.baseRotation));

        auto* lod = type->getLod(0);
        if (lod) {
            auto* mesh = lod->m_mesh;
            if (mesh) {
                transform.setVolume(mesh->getAABB().getVolume());
                baseTransform = baseTransform * mesh->m_transform;
            }
        }

        transform.setBaseTransform(baseTransform);

        node->m_camera = l.m_cameraLoader.createCamera(data.camera);
        node->m_light = l.m_lightLoader.createLight(data.light, cloneIndex, tile);
        node->m_generator = l.m_generatorLoader.createGenerator(
            data.generator,
            m_materials,
            type);

        node->m_particleGenerator = l.m_particleLoader.createParticle(
            data.particle,
            m_materials);

        if (type->m_entityType == mesh::EntityType::text) {
            auto fontId = resolveFont(typeHandle, data.text);
            auto generator = std::make_unique<TextGenerator>();
            generator->setFontId(fontId);
            generator->setText(data.text.text);
            node->m_generator = std::move(generator);
        }

        l.m_scriptLoader.createScript(
            rootId,
            node->getId(),
            data.script);

        {
            type->setCustomMaterial(
                l.m_customMaterialLoader.createCustomMaterial(
                    data.customMaterial,
                    cloneIndex,
                    tile));
            MeshTypeRegistry::get().registerCustomMaterial(typeHandle);
        }

        return handle;
    }

    void SceneLoader::assignFlags(
        const EntityCloneData& data,
        pool::TypeHandle typeHandle)
    {
        auto* type = typeHandle.toType();

        mesh::NodeRenderFlags& flags = type->m_flags;

        flags.gbuffer = data.programName.starts_with("g_");

        // Rigged model
        {
            flags.useBones = data.findRenderFlag("use_bones", flags.useBones);

            // NOTE KI bones are *required* if using animation
            flags.useAnimation = data.findRenderFlag("use_animation", flags.useAnimation);
            if (flags.useAnimation) {
                flags.useBones = true;
            }

            // NOTE KI no bones debug if no bones
            flags.useBonesDebug = data.findRenderFlag("use_bones_debug", flags.useBonesDebug);
            if (!flags.useBones) {
                flags.useBonesDebug = false;
            }
        }

        flags.preDepth = data.findRenderFlag("pre_depth", flags.preDepth);
        flags.gbuffer = data.findRenderFlag("gbuffer", flags.gbuffer);
        flags.alpha = data.findRenderFlag("alpha", flags.alpha);

        {
            const auto& e = data.renderFlags.find("blend");
            if (e != data.renderFlags.end()) {
                flags.blend = e->second;
                // NOTE KI alpha MUST BE true if blend
                if (flags.blend) {
                    KI_WARN(fmt::format("BLEND requires alpha (enabled alpha): id={}, desc={}", data.baseId, data.desc));
                    flags.alpha = true;
                }
                // NOTE KI blend CANNOT be gbuffer
                if (flags.blend && flags.gbuffer) {
                    KI_ERROR(fmt::format("GBUFFER vs. BLEND mismatch (disabled blend): id={}, desc={}", data.baseId, data.desc));
                    // NOTE KI turning off blend; shader is designed for gbuffer
                    flags.blend = false;
                }
            }
        }
        {
            const auto& e = data.renderFlags.find("blend_oit");
            if (e != data.renderFlags.end()) {
                flags.blendOIT = e->second;
                // NOTE KI alpha MUST BE true if blend
                if (flags.blendOIT) {
                    KI_WARN(fmt::format("BLEND requires alpha (enabled alpha): id={}, desc={}", data.baseId, data.desc));
                    flags.alpha = true;
                }
            }
        }

        flags.renderBack = data.findRenderFlag("render_back", flags.renderBack);
        flags.noFrustum = data.findRenderFlag("no_frustum", flags.noFrustum);
        flags.noShadow = data.findRenderFlag("no_shadow", flags.noShadow);
        flags.noSelect = data.findRenderFlag("no_select", flags.noSelect);
        flags.noReflect = data.findRenderFlag("no_reflect", flags.noReflect);
        flags.noRefract = data.findRenderFlag("no_refract", flags.noRefract);
        flags.mirror = data.findRenderFlag("mirror", flags.mirror);
        flags.water = data.findRenderFlag("water", flags.water);
        flags.wireframe = data.findRenderFlag("wireframe", flags.wireframe);
        flags.cubeMap = data.findRenderFlag("cube_map", flags.cubeMap);
        flags.effect = data.findRenderFlag("effect", flags.effect);
        flags.billboard = data.findRenderFlag("billboard", flags.billboard);
        flags.tessellation = data.findRenderFlag("tessellation", flags.tessellation);

        {
            const auto& e = data.renderFlags.find("static_bounds");
            if (e != data.renderFlags.end()) {
                flags.staticBounds = e->second;
                flags.physics = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("dynamic_bounds");
            if (e != data.renderFlags.end()) {
                flags.dynamicBounds = e->second;
                flags.physics = e->second;
            }
        }

        if (data.physics.enabled || flags.staticBounds || flags.dynamicBounds) {
            flags.physics = true;
        }
    }

    void SceneLoader::loadMeta(
        const YAML::Node& node,
        MetaData& data) const
    {
        data.name = "<noname>";
        //data.modelsDir = assets.modelsDir;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "name") {
                data.name= readString(v);
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
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!data.enabled) return;

        const auto& repeat = data.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    validateEntityCloneRepeat(
                        rootId,
                        entityRoot,
                        data,
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
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset,
        int pass,
        int& errorCount,
        std::map<ki::node_id, std::string>& collectedIds)
    {
        if (!data.enabled) return;

        if (pass == 0) {
            if (data.name == "Linden tree forest - part 1")
                int x = 0;

            ki::node_id sid;
            std::string resolvedSID;
            {
                auto [k, v] = resolveId(
                    data.baseId,
                    cloneIndex, tile,
                    data.baseId.m_path == entityRoot.base.baseId.m_path);
                sid = k;
                resolvedSID = v;

                if (!sid) {
                    auto [k, v] = resolveId(
                        { data.name },
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
            if (!data.parentBaseId.empty()) {
                auto [sid, resolvedSID] = resolveId(
                    data.parentBaseId,
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

    const Sprite* SceneLoader::findSprite(
        std::string_view name) const
    {
        const auto& it = std::find_if(
            m_sprites.cbegin(),
            m_sprites.cend(),
            [&name](const auto& m) { return m.sprite.m_name == name; });
        return it != m_sprites.end() ? &(it->sprite) : nullptr;
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
