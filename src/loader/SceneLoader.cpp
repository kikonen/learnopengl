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

#include "asset/Material.h"
#include "asset/Sprite.h"
#include "asset/Shape.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "mesh/ModelMesh.h"
#include "mesh/QuadMesh.h"
#include "mesh/SpriteMesh.h"
#include "mesh/TextMesh.h"

#include "text/TextMaterial.h"
#include "text/TextDraw.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "model/Node.h"

#include "generator/GridGenerator.h"
#include "generator/TerrainGenerator.h"
#include "generator/AsteroidBeltGenerator.h"
#include "generator/TextGenerator.h"

#include "event/Dispatcher.h"

#include "mesh/MeshType.h"

#include "registry/Registry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/ProgramRegistry.h"

#include <engine/AsyncLoader.h>


namespace {
    const std::string QUAD_MESH_NAME{ "quad" };
}

namespace loader {
    SceneLoader::SceneLoader(
        Context ctx)
        : BaseLoader(ctx),
        m_rootLoader(ctx),
        m_scriptLoader(ctx),
        m_skyboxLoader(ctx),
        m_volumeLoader(ctx),
        m_cubeMapLoader(ctx),
        m_fontLoader(ctx),
        m_materialLoader(ctx),
        m_customMaterialLoader(ctx),
        m_spriteLoader(ctx),
        m_cameraLoader(ctx),
        m_lightLoader(ctx),
        m_audioLoader(ctx),
        m_controllerLoader(ctx),
        m_generatorLoader(ctx),
        m_physicsLoader(ctx),
        m_entityLoader(ctx)
    {
        m_defaultMaterial = Material::createMaterial(BasicMaterial::basic);
    }

    SceneLoader::~SceneLoader()
    {
        KI_INFO(fmt::format("SCENE_FILE: delete - ctx={}", m_ctx.str()));
    }

    void SceneLoader::destroy()
    {}

    bool SceneLoader::isRunning()
    {
        std::lock_guard<std::mutex> lock(m_ready_lock);
        return m_runningCount > 0 || m_pendingCount > 0;
    }

    void SceneLoader::prepare(
        std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = registry->m_dispatcher;

        m_fontLoader.setRegistry(registry);
        m_materialLoader.setRegistry(registry);
        m_rootLoader.setRegistry(registry);
        m_scriptLoader.setRegistry(registry);
        m_skyboxLoader.setRegistry(registry);
        m_volumeLoader.setRegistry(registry);
        m_cubeMapLoader.setRegistry(registry);
        m_audioLoader.setRegistry(registry);
        m_physicsLoader.setRegistry(registry);
        m_entityLoader.setRegistry(registry);
    }

    void SceneLoader::load()
    {
        if (!util::fileExists(m_ctx.m_fullPath)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", m_ctx.str()) };
        }

        std::lock_guard<std::mutex> lock(m_ready_lock);
        m_runningCount++;

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this]() {
            std::ifstream fin(this->m_ctx.m_fullPath);
            YAML::Node doc = YAML::Load(fin);

            loadMeta(doc["meta"], m_meta);

            m_skyboxLoader.loadSkybox(doc["skybox"], m_skybox);

            m_fontLoader.loadFonts(doc["fonts"], m_fonts);
            m_materialLoader.loadMaterials(doc["materials"], m_materials);
            m_spriteLoader.loadSprites(doc["sprites"], m_sprites);

            m_rootLoader.loadRoot(doc["root"], m_root);
            m_scriptLoader.loadScriptEngine(doc["script"], m_scriptEngineData);

            m_entityLoader.loadEntities(
                doc["entities"],
                m_entities,
                m_materialLoader,
                m_customMaterialLoader,
                m_spriteLoader,
                m_cameraLoader,
                m_lightLoader,
                m_audioLoader,
                m_controllerLoader,
                m_generatorLoader,
                m_physicsLoader,
                m_scriptLoader);

            attach(m_root);

            std::lock_guard<std::mutex> lock(m_ready_lock);
            m_runningCount--;
        });
    }

    void SceneLoader::loadedEntity(
        const EntityData& data,
        bool success)
    {
        std::lock_guard<std::mutex> lock(m_ready_lock);

        m_pendingCount--;

        KI_INFO_OUT(fmt::format(
            "LOADED: entity={}, success={}, pending={}",
            data.base.name, success, m_pendingCount));

        if (m_pendingCount > 0) return;

        // NOTE KI event will be put queue *AFTER* entity attach events
        // => should they should be fully attached in scene at this point
        // => worker will trigger event into UI thread after processing all updates
        {
            event::Event evt { event::Type::scene_loaded };
            m_dispatcher->send(evt);
        }
    }

    void SceneLoader::attach(
        const RootData& root)
    {
        m_rootLoader.attachRoot(root);
        m_scriptLoader.createScriptEngine(root.rootId, m_scriptEngineData);

        m_skyboxLoader.attachSkybox(root.rootId, m_skybox);
        m_volumeLoader.attachVolume(root.rootId);
        m_cubeMapLoader.attachCubeMap(root.rootId);

        m_fontLoader.createFonts(m_fonts);

        {
            std::lock_guard<std::mutex> lock(m_ready_lock);

            m_pendingCount = 0;
            for (const auto& entity : m_entities) {
                if (attachEntity(root.rootId, entity)) {
                    m_pendingCount++;
                    KI_INFO_OUT(fmt::format("START: entity={}, pending={}", entity.base.name, m_pendingCount));
                }
            }

            KI_INFO_OUT(fmt::format("TOTAL: pending={}", m_pendingCount));
        }
    }

    bool SceneLoader::attachEntity(
        const uuids::uuid& rootId,
        const EntityData& data)
    {
        if (!data.base.enabled) {
            return false;
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this, &rootId, &data]() {
            try {
                if (data.clones.empty()) {
                    const mesh::MeshType* type{ nullptr };
                    attachEntityClone(type, rootId, data, data.base, false, 0);
                }
                else {
                    const mesh::MeshType* type{ nullptr };

                    int cloneIndex = 0;
                    for (auto& cloneData : data.clones) {
                        if (!*m_ctx.m_alive) return;
                        type = attachEntityClone(type, rootId, data, cloneData, true, cloneIndex);
                        if (!data.base.cloneMesh)
                            type = nullptr;
                        cloneIndex++;
                    }
                }
                loadedEntity(data, true);
            }
            catch (const std::runtime_error& ex) {
                loadedEntity(data, false);
                throw ex;
            }
        });

        return true;
    }

    const mesh::MeshType* SceneLoader::attachEntityClone(
        const mesh::MeshType* type,
        const uuids::uuid& rootId,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex)
    {
        if (!*m_ctx.m_alive) return type;

        if (!data.enabled) {
            return type;
        }

        const auto& repeat = data.repeat;

        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    if (!*m_ctx.m_alive) return type;

                    const glm::uvec3 tile = { x, y, z };
                    const glm::vec3 tilePositionOffset{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                    if (x > 0 && z > 0)
                        int zz = 0;

                    type = attachEntityCloneRepeat(
                        type,
                        rootId,
                        entity,
                        data,
                        cloned,
                        cloneIndex,
                        tile,
                        tilePositionOffset);

                    if (!entity.base.cloneMesh)
                        type = nullptr;
                }
            }
        }

        return type;
    }

    const mesh::MeshType* SceneLoader::attachEntityCloneRepeat(
        const mesh::MeshType* type,
        const uuids::uuid& rootId,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset)
    {
        if (!*m_ctx.m_alive) return type;

        if (!data.enabled) {
            return type;
        }

        // NOTE KI overriding material in clones is *NOT* supported"
        if (!type) {
            type = createType(
                data,
                tile);
            if (!type) return type;
        }

        if (!*m_ctx.m_alive) return type;

        auto node = createNode(
            type, rootId, data,
            cloned, cloneIndex, tile,
            data.clonePositionOffset,
            tilePositionOffset);

        {
            // NOTE KI no id for clones; would duplicate base id => conflicts
            // => except if clone defines own ID
            const auto uuid = resolveUUID(data.idBase, cloneIndex, tile);

            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            evt.body.node.uuid = uuid;
            {
                if (data.parentIdBase.empty()) {
                    evt.body.node.parentUUID = rootId;
                }
                else {
                    evt.body.node.parentUUID = resolveUUID(
                        data.parentIdBase,
                        cloneIndex,
                        tile);
                }
            }
            m_dispatcher->send(evt);
        }

        if (data.active) {
            event::Event evt { event::Type::node_activate };
            evt.body.node.target = node;
            m_dispatcher->send(evt);
        }

        if (data.selected) {
            event::Event evt { event::Type::node_select };
            evt.body.node.target = node;
            m_dispatcher->send(evt);
        }

        if (data.camera.isDefault) {
            event::Event evt { event::Type::camera_activate };
            evt.body.node.target = node;
            m_dispatcher->send(evt);
        }

        for (auto& controllerData : data.controllers) {
            if (!controllerData.enabled) continue;

            auto* controller = m_controllerLoader.createController(controllerData, node);

            event::Event evt { event::Type::controller_add };
            evt.body.control = {
                .target = node->m_id,
                .controller = controller
            };
            m_dispatcher->send(evt);
        }

        // try anim event
        //if (!entity.isRoot && !type->m_flags.water && !type->m_flags.tessellation && !type->m_flags.noShadow)
        if (data.name == "Cow")
        {
            event::Event evt { event::Type::animate_rotate };
            evt.body.animate = {
                .target = node->m_id,
                .duration = 20,
                .data = { 0, 1.f, 0 },
                .data2 = { 360.f, 0.f, 0.f },
            };
            m_dispatcher->send(evt);
        }

        {
            m_audioLoader.createAudio(data.audio, node->m_id);
            m_physicsLoader.createObject(data.physics, node->m_id);
        }

        return type;
    }

    const mesh::MeshType* SceneLoader::createType(
        const EntityCloneData& data,
        const glm::uvec3& tile)
    {
        auto* type = m_registry->m_typeRegistry->registerType(data.name);
        assignFlags(data, type);

        type->m_priority = data.priority;

        if (data.instanced) {
            type->m_flags.instanced = true;
        }

        if (data.type == mesh::EntityType::origo) {
            type->m_flags.invisible = true;
            type->m_entityType = mesh::EntityType::origo;
        } else
        {
            resolveMaterial(type, data);
            resolveSprite(type, data);
            resolveMesh(type, data, tile);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (data.type != mesh::EntityType::container) {
                if (!type->getMesh()) {
                    KI_WARN(fmt::format(
                        "SCENE_FILEIGNORE: NO_MESH id={} ({})",
                        data.idBase, data.name));
                    return nullptr;
                }
            }

            modifyMaterials(type, data);
            resolveProgram(type, data);
        }

        return type;
    }

    void SceneLoader::resolveProgram(
        mesh::MeshType* type,
        const EntityCloneData& data)
    {
        bool useTBN = false;
        bool useParallax = false;
        bool useDudvTex = false;
        bool useHeightTex = false;
        bool useDisplacementTex = false;
        bool useNormalTex = false;
        bool useCubeMap = false;
        bool useNormalPattern = false;

        type->modifyMaterials([
            this,
                &useNormalTex, &useCubeMap, &useDudvTex, &useHeightTex, &useDisplacementTex, &useNormalPattern, &useParallax,
                &data
            ](Material& m) {
                useDudvTex |= m.hasTex(MATERIAL_DUDV_MAP_IDX);
                useHeightTex |= m.hasTex(MATERIAL_HEIGHT_MAP_IDX);
                useDisplacementTex |= m.hasTex(MATERIAL_DISPLACEMENT_MAP_IDX);
                useNormalTex |= m.hasTex(MATERIAL_NORMAL_MAP_IDX);
                useCubeMap |= 1.0 - m.reflection - m.refraction < 1.0;
                useNormalPattern |= m.pattern > 0;
                useParallax |= m.hasTex(MATERIAL_DISPLACEMENT_MAP_IDX) && m.parallaxDepth > 0;
                if (!useParallax) {
                    m.parallaxDepth = 0.f;
                }
            });
        useTBN = useNormalTex || useDudvTex || useDisplacementTex;

        if (!data.programName.empty()) {
            std::map<std::string, std::string, std::less<>> definitions;
            for (const auto& [k, v] : data.programDefinitions) {
                definitions[k] = v;
            }

            std::map<std::string, std::string, std::less<>> depthDefinitions;
            bool useDepth = type->m_flags.depth;

            if (type->m_flags.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
                useDepth = false;
            }
            if (type->m_flags.blend) {
                definitions[DEF_USE_BLEND] = "1";
                useDepth = false;
            }
            if (type->m_flags.blendOIT) {
                definitions[DEF_USE_BLEND_OIT] = "1";
                useDepth = false;
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
            if (useHeightTex) {
                definitions[DEF_USE_HEIGHT_TEX] = "1";
            }
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

            type->m_program = m_registry->m_programRegistry->getProgram(
                data.programName,
                false,
                data.geometryType,
                definitions);

            if (useDepth) {
                type->m_depthProgram = m_registry->m_programRegistry->getProgram(
                    data.depthProgramName,
                    false,
                    "",
                    depthDefinitions);
            }
        }
    }

    text::font_id SceneLoader::resolveFont(
        const mesh::MeshType* type,
        const TextData& data) const
     {
        auto* font = findFont(data.font);
        return font ? font->id : 0;
    }

    void SceneLoader::resolveMaterial(
        mesh::MeshType* type,
        const EntityCloneData& data)
    {
        // NOTE KI need to create copy *IF* modifiers
        // TODO KI should make copy *ALWAYS* for safety
        const Material* material = nullptr;

        if (!data.materialName.empty()) {
            material = findMaterial(data.materialName);
        }

        if (!material) {
            material = &m_defaultMaterial;
        }

        auto& materialVBO = type->m_materialVBO;
        materialVBO->setDefaultMaterial(*material, true, data.forceMaterial);
    }

    void SceneLoader::modifyMaterials(
        mesh::MeshType* type,
        const EntityCloneData& data)
    {
        type->modifyMaterials([this, &data](Material& m) {
            m_materialLoader.modifyMaterial(m, data.materialModifiers);
            m.loadTextures(m_assets);
        });
    }

    void SceneLoader::resolveSprite(
        mesh::MeshType* type,
        const EntityCloneData& data)
    {
        const Sprite* sprite{ nullptr };

        if (!data.spriteName.empty()) {
            sprite = findSprite(data.spriteName);
        }

        if (sprite) {
            type->m_sprite = std::make_unique<Sprite>(*sprite);
        }
    }

    void SceneLoader::resolveMesh(
        mesh::MeshType* type,
        const EntityCloneData& data,
        const glm::uvec3& tile)
    {
        // NOTE KI materials MUST be resolved before loading mesh
        if (data.type == mesh::EntityType::model) {
            auto future = m_registry->m_modelRegistry->getMesh(
                data.meshName,
                m_assets.modelsDir,
                data.meshPath);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = mesh::EntityType::model;

            KI_INFO(fmt::format(
                "SCENE_FILE ATTACH: id={}, type={}",
                data.idBase, type->str()));
        }
        else if (data.type == mesh::EntityType::quad) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_assets.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = mesh::EntityType::quad;
        }
        else if (data.type == mesh::EntityType::billboard) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_assets.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = mesh::EntityType::billboard;
        }
        else if (data.type == mesh::EntityType::sprite) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_assets.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = mesh::EntityType::sprite;
        }
        else if (data.type == mesh::EntityType::text) {
            type->m_entityType = mesh::EntityType::text;
            auto mesh = std::make_unique<mesh::TextMesh>();
            type->setMesh(std::move(mesh), true);
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

    Node* SceneLoader::createNode(
        const mesh::MeshType* type,
        const uuids::uuid& rootId,
        const EntityCloneData& data,
        const bool cloned,
        const int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& clonePositionOffset,
        const glm::vec3& tilePositionOffset)
    {
        Node* node = new Node(type);

        node->setCloneIndex(cloneIndex);
        //node->setTile(tile);

        glm::vec3 pos = data.position + clonePositionOffset + tilePositionOffset;

        auto& transform = node->modifyTransform();
        transform.setPosition(pos);
        transform.setBaseRotation(util::degreesToQuat(data.baseRotation));
        transform.setQuatRotation(util::degreesToQuat(data.rotation));
        transform.setScale(data.scale);
        transform.setFront(data.front);

        auto mesh = type->getMesh();
        if (mesh) {
            transform.setVolume(mesh->getAABB().getVolume());
        }

        node->m_camera = m_cameraLoader.createCamera(data.camera);
        node->m_light = m_lightLoader.createLight(data.light, cloneIndex, tile);
        node->m_generator = m_generatorLoader.createGenerator(data.generator, node);

        if (type->m_entityType == mesh::EntityType::text) {
            auto fontId = resolveFont(type, data.text);
            auto generator = std::make_unique<TextGenerator>();
            generator->setFontId(fontId);
            generator->setText(data.text.text);
            node->m_generator = std::move(generator);
        }

        m_scriptLoader.createScript(
            rootId,
            node->m_id,
            data.script);

        {
            auto* t = m_registry->m_typeRegistry->modifyType(type->m_id);
            t->setCustomMaterial(
                m_customMaterialLoader.createCustomMaterial(
                    data.customMaterial,
                    cloneIndex,
                    tile));
            m_registry->m_typeRegistry->registerCustomMaterial(type->m_id);
            type = t;
            node->m_type = type;
        }

        return node;
    }

    void SceneLoader::assignFlags(
        const EntityCloneData& data,
        mesh::MeshType* type)
    {
        mesh::NodeRenderFlags& flags = type->m_flags;

        flags.gbuffer = data.programName.starts_with("g_");

        {
            const auto& e = data.renderFlags.find("depth");
            if (e != data.renderFlags.end()) {
                flags.depth = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("gbuffer");
            if (e != data.renderFlags.end()) {
                flags.gbuffer = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("alpha");
            if (e != data.renderFlags.end()) {
                flags.alpha = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("blend");
            if (e != data.renderFlags.end()) {
                flags.blend = e->second;
                // NOTE KI alpha MUST BE true if blend
                if (flags.blend) {
                    KI_WARN(fmt::format("BLEND requires alpha (enabled alpha): {}", data.name));
                    flags.alpha = true;
                }
                // NOTE KI blend CANNOT be gbuffer
                if (flags.blend && flags.gbuffer) {
                    KI_ERROR(fmt::format("GBUFFER vs. BLEND mismatch (disabled blend): {}", data.name));
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
                    KI_WARN(fmt::format("BLEND requires alpha (enabled alpha): {}", data.name));
                    flags.alpha = true;
                }
            }
        }
        {
            const auto& e = data.renderFlags.find("render_back");
            if (e != data.renderFlags.end()) {
                flags.renderBack = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("no_frustum");
            if (e != data.renderFlags.end()) {
                flags.noFrustum = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("no_shadow");
            if (e != data.renderFlags.end()) {
                flags.noShadow = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("no_select");
            if (e != data.renderFlags.end()) {
                flags.noSelect = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("no_reflect");
            if (e != data.renderFlags.end()) {
                flags.noReflect = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("no_refract");
            if (e != data.renderFlags.end()) {
                flags.noRefract = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("mirror");
            if (e != data.renderFlags.end()) {
                flags.mirror = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("water");
            if (e != data.renderFlags.end()) {
                flags.water = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("wireframe");
            if (e != data.renderFlags.end()) {
                flags.wireframe = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("cube_map");
            if (e != data.renderFlags.end()) {
                flags.cubeMap = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("effect");
            if (e != data.renderFlags.end()) {
                flags.effect = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("tessellation");
            if (e != data.renderFlags.end()) {
                flags.tessellation = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("static_physics");
            if (e != data.renderFlags.end()) {
                flags.staticPhysics = e->second;
            }
        }
        {
            const auto& e = data.renderFlags.find("enforce_bounds");
            if (e != data.renderFlags.end()) {
                flags.enforceBounds = e->second;
            }
        }
    }

    void SceneLoader::loadMeta(
        const YAML::Node& node,
        MetaData& data) const
    {
        data.name = "<noname>";
        //data.modelsDir = m_assets.modelsDir;

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

    const Material* SceneLoader::findMaterial(
        std::string_view name) const
    {
        const auto& it = std::find_if(
            m_materials.cbegin(),
            m_materials.cend(),
            [&name](const auto& m) { return m.material.m_name == name && !m.material.m_default; });
        return it != m_materials.end() ? &(it->material) : nullptr;
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
