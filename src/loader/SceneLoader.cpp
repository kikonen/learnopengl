#include "SceneLoader.h"

#include <string>
#include <vector>
#include <fstream>
#include <string>

#include <fmt/format.h>

#include "util/Util.h"
#include "util/glm_format.h"

#include "ki/yaml.h"

#include "asset/Material.h"
#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"
#include "asset/SpriteMesh.h"
#include "asset/Sprite.h"
#include "asset/Shape.h"
#include "asset/Program.h"
#include "asset/Shader.h"
#include "asset/TextMaterial.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "model/Node.h"

#include "physics/Object.h"
#include "physics/PhysicsEngine.h"

#include "generator/GridGenerator.h"
#include "generator/TerrainGenerator.h"
#include "generator/AsteroidBeltGenerator.h"

#include "controller/CameraController.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/MeshType.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"

#include <engine/AsyncLoader.h>


namespace {
    const std::string QUAD_MESH_NAME{ "quad" };
}

namespace loader {
    SceneLoader::SceneLoader(
        Context ctx)
        : BaseLoader(ctx),
        m_rootLoader(ctx),
        m_skyboxLoader(ctx),
        m_volumeLoader(ctx),
        m_cubeMapLoader(ctx),
        m_materialLoader(ctx),
        m_customMaterialLoader(ctx),
        m_spriteLoader(ctx),
        m_cameraLoader(ctx),
        m_lightLoader(ctx),
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

    void SceneLoader::prepare(
        std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = registry->m_dispatcher;

        m_materialLoader.m_registry = m_registry;

        m_rootLoader.m_registry = m_registry;
        m_rootLoader.m_dispatcher = m_dispatcher;

        m_skyboxLoader.m_registry = m_registry;
        m_skyboxLoader.m_dispatcher = m_dispatcher;

        m_volumeLoader.m_registry = m_registry;
        m_volumeLoader.m_dispatcher = m_dispatcher;

        m_cubeMapLoader.m_registry = m_registry;
        m_cubeMapLoader.m_dispatcher = m_dispatcher;
    }

    void SceneLoader::load()
    {
        if (!util::fileExists(m_ctx.m_filename)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", m_ctx.str()) };
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this]() {
            std::ifstream fin(this->m_ctx.m_filename);
            YAML::Node doc = YAML::Load(fin);

            loadMeta(doc, m_meta);

            m_skyboxLoader.loadSkybox(doc, m_skybox);
            m_materialLoader.loadMaterials(doc, m_materials);
            m_spriteLoader.loadSprites(doc, m_sprites);

            m_rootLoader.loadRoot(doc, m_root);

            m_entityLoader.loadEntities(
                doc,
                m_entities,
                m_materialLoader,
                m_customMaterialLoader,
                m_spriteLoader,
                m_cameraLoader,
                m_lightLoader,
                m_controllerLoader,
                m_generatorLoader,
                m_physicsLoader);

            attach(m_root);
        });
    }

    void SceneLoader::loadedEntity(const EntityData& data)
    {
        std::lock_guard<std::mutex> lock(m_ready_lock);

        if (--m_pendingCount > 0) return;

        // NOTE KI event will be put queue *AFTER* entity attach events
        // => should they should be fully attached in scene at this point
        event::Event evt { event::Type::scene_loaded };
        m_dispatcher->send(evt);
    }

    void SceneLoader::attach(
        const RootData& root)
    {
        m_rootLoader.attachRoot(root);

        m_skyboxLoader.attachSkybox(root.rootId, m_skybox);
        m_volumeLoader.attachVolume(root.rootId);
        m_cubeMapLoader.attachCubeMap(root.rootId);

        m_pendingCount = m_entities.size();
        for (const auto& entity : m_entities) {
            attachEntity(root.rootId, entity);
        }
    }

    void SceneLoader::attachEntity(
        const uuids::uuid& rootId,
        const EntityData& data)
    {
        if (!data.base.enabled) {
            return;
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this, &rootId, &data]() {
            if (data.clones.empty()) {
                MeshType* type{ nullptr };
                attachEntityClone(type, rootId, data, data.base, false, 0);
            }
            else {
                MeshType* type{ nullptr };

                int cloneIndex = 0;
                for (auto& cloneData : data.clones) {
                    if (!*m_ctx.m_alive) return;
                    type = attachEntityClone(type, rootId, data, cloneData, true, cloneIndex);
                    if (!data.base.cloneMesh)
                        type = nullptr;
                    cloneIndex++;
                }
            }

            loadedEntity(data);
        });
    }

    MeshType* SceneLoader::attachEntityClone(
        MeshType* type,
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

    MeshType* SceneLoader::attachEntityCloneRepeat(
        MeshType* type,
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
                entity.isRoot,
                data,
                tile);
            if (!type) return type;
        }

        if (!*m_ctx.m_alive) return type;

        auto node = createNode(
            type, rootId, data,
            cloned, cloneIndex, tile,
            data.clonePositionOffset,
            tilePositionOffset,
            entity.isRoot);

        {
            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            if (!entity.isRoot) {
                if (data.parentIdBase.empty()) {
                    evt.body.node.parentId = rootId;
                }
                else {
                    evt.body.node.parentId = resolveUUID(
                        data.parentIdBase,
                        cloneIndex,
                        tile);
                }
            }
            m_dispatcher->send(evt);
        }

        if (data.selected) {
            event::Event evt { event::Type::node_select };
            evt.body.node.target = node;
            m_dispatcher->send(evt);
        }

        if (data.controller.enabled)
        {
            auto* controller = m_controllerLoader.createController(data.controller, node);

            event::Event evt { event::Type::controller_add };
            evt.body.control = {
                .target = node->m_objectID,
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
                .target = node->m_objectID,
                .duration = 20,
                .data = { 0, 360.f, 0 },
            };
            m_dispatcher->send(evt);
        }

        return type;
    }

    MeshType* SceneLoader::createType(
        bool isRoot,
        const EntityCloneData& data,
        const glm::uvec3& tile)
    {
        auto type = m_registry->m_typeRegistry->getType(data.name);
        assignFlags(data, type);

        type->m_priority = data.priority;
        type->m_script = data.script;

        if (isRoot) {
            type->m_flags.invisible = true;
            type->m_entityType = EntityType::origo;
        }

        if (data.instanced) {
            type->m_flags.instanced = true;
        }

        if (data.type == EntityType::origo) {
            type->m_flags.invisible = true;
            type->m_entityType = EntityType::origo;
        } else
        {
            resolveMaterial(type, data);
            resolveSprite(type, data);
            resolveMesh(type, data, tile);

            // NOTE KI container does not have mesh itself, but it can setup
            // material & program for contained nodes
            if (data.type != EntityType::container) {
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
        MeshType* type,
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
            if (useDudvTex) {
                definitions[DEF_USE_DUDV_TEX] = "1";
            }
            if (useHeightTex) {
                definitions[DEF_USE_HEIGHT_TEX] = "1";
            }
            if (useDisplacementTex) {
                definitions[DEF_USE_DISPLACEMENT_TEX] = "1";
            }
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

    void SceneLoader::resolveMaterial(
        MeshType* type,
        const EntityCloneData& data)
    {
        // NOTE KI need to create copy *IF* modifiers
        // TODO KI should make copy *ALWAYS* for safety
        Material* material = nullptr;

        if (!data.materialName.empty()) {
            material = findMaterial(data.materialName);
        }

        if (!material) {
            material = &m_defaultMaterial;
        }

        auto& materialVBO = type->m_materialVBO;

        materialVBO.m_defaultMaterial = *material;
        materialVBO.m_useDefaultMaterial = true;
        materialVBO.m_forceDefaultMaterial = data.forceMaterial;
    }

    void SceneLoader::modifyMaterials(
        MeshType* type,
        const EntityCloneData& data)
    {
        type->modifyMaterials([this, &data](Material& m) {
            m_materialLoader.modifyMaterial(m, data.materialModifiers);
            m.loadTextures(m_assets);
        });
    }

    void SceneLoader::resolveSprite(
        MeshType* type,
        const EntityCloneData& data)
    {
        Sprite* sprite{ nullptr };

        if (!data.spriteName.empty()) {
            sprite = findSprite(data.spriteName);
        }

        if (sprite) {
            type->m_sprite = *sprite;
        }
    }

    void SceneLoader::resolveMesh(
        MeshType* type,
        const EntityCloneData& data,
        const glm::uvec3& tile)
    {
        // NOTE KI materials MUST be resolved before loading mesh
        if (data.type == EntityType::model) {
            auto future = m_registry->m_modelRegistry->getMesh(
                data.meshName,
                m_assets.modelsDir,
                data.meshPath);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = EntityType::model;

            KI_INFO(fmt::format(
                "SCENE_FILE ATTACH: id={}, type={}",
                data.idBase, type->str()));
        }
        else if (data.type == EntityType::quad) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_assets.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = EntityType::quad;
        }
        else if (data.type == EntityType::billboard) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_assets.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = EntityType::billboard;
        }
        else if (data.type == EntityType::sprite) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_assets.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = EntityType::sprite;
        }
        else if (data.type == EntityType::terrain) {
            type->m_entityType = EntityType::terrain;
        }
        else if (data.type == EntityType::container) {
            // NOTE KI generator takes care of actual work
            type->m_entityType = EntityType::container;
            type->m_flags.invisible = true;
        }
        else {
            // NOTE KI root/origo/unknown; don't render, just keep it in hierarchy
            type->m_entityType = EntityType::origo;
            type->m_flags.invisible = true;
        }
    }

    Node* SceneLoader::createNode(
        MeshType* type,
        const uuids::uuid& rootId,
        const EntityCloneData& data,
        const bool cloned,
        const int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& clonePositionOffset,
        const glm::vec3& tilePositionOffset,
        const bool isRoot)
    {
        Node* node = new Node(type);

        node->setCloneIndex(cloneIndex);
        //node->setTile(tile);

        glm::vec3 pos = data.position + clonePositionOffset + tilePositionOffset;

        // NOTE KI no id for clones; would duplicate base id => conflicts
        // => except if clone defines own ID
        if (isRoot) {
            node->m_id = rootId;
        } else if (!cloned) {
            node->m_id = resolveUUID(data.idBase, cloneIndex, tile);
        }

        node->setPosition(pos);
        node->setRotation(data.rotation);
        node->setScale(data.scale);

        node->setFront(data.front);

        auto mesh = type->getMesh();
        if (mesh) {
            node->setVolume(mesh->getAABB().getVolume());
        }

        node->m_camera = m_cameraLoader.createCamera(data.camera);
        node->m_light = m_lightLoader.createLight(data.light, cloneIndex, tile);
        node->m_generator = m_generatorLoader.createGenerator(data.generator, node);
        node->m_physics = m_physicsLoader.createObject(data.physics, node);

        type->setCustomMaterial(
            m_customMaterialLoader.createCustomMaterial(data.customMaterial, cloneIndex, tile));

        return node;
    }

    void SceneLoader::assignFlags(
        const EntityCloneData& data,
        MeshType* type)
    {
        NodeRenderFlags& flags = type->m_flags;

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
            const auto& e = data.renderFlags.find("physics");
            if (e != data.renderFlags.end()) {
                flags.physics = e->second;
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
        const YAML::Node& doc,
        MetaData& data) const
    {
        data.name = "<noname>";
        //data.modelsDir = m_assets.modelsDir;

        auto& node = doc["meta"];

        if (!node) return;

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

    Material* SceneLoader::findMaterial(
        std::string_view name)
    {
        const auto& it = std::find_if(
            m_materials.begin(),
            m_materials.end(),
            [&name](MaterialData& m) { return m.material.m_name == name && !m.material.m_default; });
        return it != m_materials.end() ? &(it->material) : nullptr;
    }

    Sprite* SceneLoader::findSprite(
        std::string_view name)
    {
        const auto& it = std::find_if(
            m_sprites.begin(),
            m_sprites.end(),
            [&name](SpriteData& m) { return m.sprite.m_name == name; });
        return it != m_sprites.end() ? &(it->sprite) : nullptr;
    }
}
