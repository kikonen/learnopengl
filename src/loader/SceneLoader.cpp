#include "SceneLoader.h"

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <string>
#include <filesystem>
#include <regex>

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
#include "controller/VolumeController.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/MeshType.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/ModelRegistry.h"

#include "scene/SkyboxMaterial.h"

#include <engine/AsyncLoader.h>


namespace {
    const std::string QUAD_MESH_NAME{ "quad" };
    const std::string SKYBOX_MESH_NAME{ "quad_skybox" };

    const std::vector<std::regex> hdriMatchers{
        std::regex(".*[\\.]hdr"),
    };
}

template <> struct fmt::formatter<loader::BaseUUID> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        return it;
    }

    template <typename FormatContext>
    auto format(const loader::BaseUUID& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        if (p.empty()) {
            return ctx.out();
        }
        else if (p.size() == 1) {
            return fmt::format_to(
                ctx.out(),
                "{}", p[0]);
        }
        else {
            return fmt::format_to(
                ctx.out(),
                "{}-{}", p[0], p[1]);
        }
    }
};

namespace loader {
    SceneLoader::SceneLoader(
        Context ctx)
        : BaseLoader(ctx),
        m_materialLoader(ctx),
        m_cameraLoader(ctx),
        m_lightLoader(ctx),
        m_controllerLoader(ctx),
        m_generatorLoader(ctx)
    {
    }

    SceneLoader::~SceneLoader()
    {
        KI_INFO(fmt::format("SCENE_FILE: delete - ctx={}", m_ctx.str()));
    }

    void SceneLoader::load(
        std::shared_ptr<Registry> registry)
    {
        m_registry = registry;
        m_dispatcher = registry->m_dispatcher;

        m_materialLoader.m_registry = registry;
        m_materialLoader.m_dispatcher = registry->m_dispatcher;

        if (!util::fileExists(m_ctx.m_filename)) {
            throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", m_ctx.str()) };
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this]() {
            std::ifstream fin(this->m_ctx.m_filename);
            YAML::Node doc = YAML::Load(fin);

            loadMeta(doc, m_meta);

            loadSkybox(doc, m_skybox);
            m_materialLoader.loadMaterials(doc);
            loadSprites(doc, m_sprites);

            loadRoot(doc, m_root);
            loadEntities(doc, m_entities);

            attach(m_skybox, m_root, m_entities, m_sprites);
        });
    }

    void SceneLoader::attach(
        SkyboxData& skybox,
        const EntityData& root,
        const std::vector<EntityData>& entities,
        std::vector<Sprite>& sprites)
    {
        attachSkybox(root, skybox);

        attachEntity(root, root, sprites);
        attachVolume(root);
        attachCubeMapCenter(root);

        for (const auto& entity : entities) {
            attachEntity(root, entity, sprites);
        }
    }

    void SceneLoader::attachSkybox(
        const EntityData& root,
        SkyboxData& data)
    {
        if (!data.valid()) return;

        auto type = m_registry->m_typeRegistry->getType("<skybox>");
        type->m_priority = data.priority;

        auto future = m_registry->m_modelRegistry->getMesh(
            SKYBOX_MESH_NAME,
            m_meta.modelsDir);
        auto* mesh = future.get();
        type->setMesh(mesh);
        type->m_entityType = EntityType::skybox;

        auto& flags = type->m_flags;

        flags.skybox = true;
        flags.wireframe = false;
        flags.renderBack = true;
        flags.noShadow = true;
        flags.noFrustum = true;
        //flags.noReflect = true;
        //flags.noRefract = true;
        flags.noDisplay = false;
        flags.noSelect = true;
        flags.gbuffer = false;// data.programName.starts_with("g_");

        type->m_program = m_registry->m_programRegistry->getProgram(data.programName);

        bool gammaCorrect = data.gammaCorrect;
        if (data.hdri) {
            gammaCorrect = false;
        }

        auto material{ std::make_unique<SkyboxMaterial>(
            data.materialName,
            gammaCorrect) };
        material->m_swapFaces = data.swapFaces;
        material->m_hdri = data.hdri;
        if (data.loadedFaces) {
            material->m_faces = data.faces;
        }
        type->setCustomMaterial(std::move(material));

        auto node = new Node(type);
        node->m_id = m_assets.skyboxUUID;

        {
            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            evt.body.node.parentId = resolveUUID(root.base.idBase, 0, { 0, 0, 0 });
            m_dispatcher->send(evt);
        }
    }

    void SceneLoader::attachVolume(
        const EntityData& root)
    {
        if (!m_assets.showVolume) return;

        auto type = m_registry->m_typeRegistry->getType("<volume>");

        auto future = m_registry->m_modelRegistry->getMesh(
            "ball_volume",
            m_meta.modelsDir);
        auto* mesh = future.get();

        type->setMesh(mesh);

        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "volume";
            material.kd = glm::vec4(0.8f, 0.8f, 0.f, 1.f);

            auto& materialVBO = type->m_materialVBO;
            materialVBO.m_defaultMaterial = material;
            materialVBO.m_useDefaultMaterial = true;
            materialVBO.m_forceDefaultMaterial = true;
            materialVBO.setMaterials({ material });
        }

        auto& flags = type->m_flags;

        flags.wireframe = true;
        flags.renderBack = true;
        flags.noShadow = true;
        flags.noFrustum = false;
        flags.noReflect = true;
        flags.noRefract = true;
        flags.noDisplay = true;
        flags.noSelect = true;
        flags.gbuffer = SHADER_VOLUME.starts_with("g_");

        type->m_program = m_registry->m_programRegistry->getProgram(SHADER_VOLUME);

        auto node = new Node(type);
        node->m_id = m_assets.volumeUUID;

        // NOTE KI m_radius = 1.73205078
        mesh->prepareVolume();

        node->setVolume(mesh->getAABB().getVolume());

        {
            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            evt.body.node.parentId = resolveUUID(root.base.idBase, 0, { 0, 0, 0 });
            m_dispatcher->send(evt);
        }
        {
            auto* controller = new VolumeController();

                event::Event evt { event::Type::controller_add };
            evt.body.control = {
                .target = node->m_objectID,
                .controller = controller
            };
            m_dispatcher->send(evt);
        }
    }

    void SceneLoader::attachCubeMapCenter(
        const EntityData& root)
    {
        if (!m_assets.showCubeMapCenter) return;

        auto type = m_registry->m_typeRegistry->getType("<cube_map>");

        auto future = m_registry->m_modelRegistry->getMesh(
            "ball_volume",
            m_meta.modelsDir);
        auto& mesh = future.get();

        type->setMesh(mesh);

        {
            auto material = Material::createMaterial(BasicMaterial::highlight);
            material.m_name = "cube_map";
            //material.kd = glm::vec4(0.f, 0.8f, 0.8f, 1.f);
            material.kd = glm::vec4(0.7516f, 0.6065f, 0.2265f, 1.f);

            auto& materialVBO = type->m_materialVBO;
            materialVBO.m_defaultMaterial = material;
            materialVBO.m_useDefaultMaterial = true;
            materialVBO.m_forceDefaultMaterial = true;
            materialVBO.setMaterials({ material });
        }

        auto& flags = type->m_flags;

        flags.wireframe = true;
        flags.renderBack = true;
        flags.noShadow = true;
        flags.noFrustum = false;
        flags.noReflect = true;
        flags.noRefract = true;
        flags.noDisplay = true;
        flags.noSelect = true;
        flags.gbuffer = SHADER_VOLUME.starts_with("g_");

        type->m_program = m_registry->m_programRegistry->getProgram(SHADER_VOLUME);

        auto node = new Node(type);
        node->m_id = m_assets.cubeMapUUID;

        //node->setScale(m_asyncLoader->assets.cubeMapFarPlane);
        node->setScale(4.f);

        // NOTE KI m_radius = 1.73205078
        mesh->prepareVolume();

        node->setVolume(mesh->getAABB().getVolume());

        {
            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            evt.body.node.parentId = resolveUUID(root.base.idBase, 0, { 0, 0, 0 });
            m_dispatcher->send(evt);
        }
    }

    void SceneLoader::attachEntity(
        const EntityData& root,
        const EntityData& data,
        std::vector<Sprite>& sprites)
    {
        if (!data.base.enabled) {
            return;
        }

        m_ctx.m_asyncLoader->addLoader(m_ctx.m_alive, [this, &root, &data, &sprites]() {
            if (data.clones.empty()) {
                MeshType* type{ nullptr };
                attachEntityClone(type, root, data, data.base, false, 0, sprites);
            }
            else {
                MeshType* type{ nullptr };

                int cloneIndex = 0;
                for (auto& cloneData : data.clones) {
                    if (!*m_ctx.m_alive) return;
                    type = attachEntityClone(type, root, data, cloneData, true, cloneIndex, sprites);
                    if (!data.base.cloneMesh)
                        type = nullptr;
                    cloneIndex++;
                }
            }
        });
    }

    MeshType* SceneLoader::attachEntityClone(
        MeshType* type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        std::vector<Sprite>& sprites)
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
                        root,
                        entity,
                        data,
                        cloned,
                        cloneIndex,
                        tile,
                        tilePositionOffset,
                        sprites);

                    if (!entity.base.cloneMesh)
                        type = nullptr;
                }
            }
        }

        return type;
    }

    MeshType* SceneLoader::attachEntityCloneRepeat(
        MeshType* type,
        const EntityData& root,
        const EntityData& entity,
        const EntityCloneData& data,
        bool cloned,
        int cloneIndex,
        const glm::uvec3& tile,
        const glm::vec3& tilePositionOffset,
        std::vector<Sprite>& sprites)
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
                tile,
                sprites);
            if (!type) return type;
        }

        if (!*m_ctx.m_alive) return type;

        auto node = createNode(
            type, root, data,
            cloned, cloneIndex, tile,
            data.clonePositionOffset,
            tilePositionOffset,
            entity.isRoot);

        {
            event::Event evt { event::Type::node_add };
            evt.body.node.target = node;
            if (!entity.isRoot) {
                evt.body.node.parentId = resolveUUID(
                    data.parentIdBase.empty() ? root.base.idBase : data.parentIdBase,
                    cloneIndex,
                    tile);
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
        const glm::uvec3& tile,
        std::vector<Sprite>& sprites)
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
            resolveSprite(type, data, sprites);
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
                m_materialLoader.modifyMaterial(m, data.materialModifiers);
                m.loadTextures(m_assets);

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

        return type;
    }

    void SceneLoader::resolveMaterial(
        MeshType* type,
        const EntityCloneData& data)
    {
        // NOTE KI need to create copy *IF* modifiers
        // TODO KI should make copy *ALWAYS* for safety
        Material* material = nullptr;

        if (!data.materialName.empty()) {
            material = m_materialLoader.find(data.materialName);
        }

        if (!material) {
            material = &m_materialLoader.m_defaultMaterial;
        }

        auto& materialVBO = type->m_materialVBO;

        materialVBO.m_defaultMaterial = *material;
        materialVBO.m_useDefaultMaterial = true;
        materialVBO.m_forceDefaultMaterial = data.forceMaterial;
    }

    void SceneLoader::resolveSprite(
        MeshType* type,
        const EntityCloneData& data,
        std::vector<Sprite>& sprites)
    {
        Sprite* sprite{ nullptr };

        if (!data.spriteName.empty()) {
            sprite = Sprite::find(data.spriteName, sprites);
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
                m_meta.modelsDir,
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
                m_meta.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = EntityType::quad;
        }
        else if (data.type == EntityType::billboard) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_meta.modelsDir);
            auto* mesh = future.get();
            type->setMesh(mesh);
            type->m_entityType = EntityType::billboard;
        }
        else if (data.type == EntityType::sprite) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME,
                m_meta.modelsDir);
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
        const EntityData& root,
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
        if (root.base.idBase != data.idBase || isRoot) {
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

        type->setCustomMaterial(createCustomMaterial(data, data.customMaterial, cloneIndex, tile));

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

    std::unique_ptr<CustomMaterial> SceneLoader::createCustomMaterial(
        const EntityCloneData& entity,
        const CustomMaterialData& data,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        if (data.type == CustomMaterialType::none) return nullptr;

        switch (data.type) {
        case CustomMaterialType::text: {
            auto material{ std::make_unique<TextMaterial>() };
            material->m_fontName = data.fontName;
            material->m_fontSize = data.fontSize;

            return material;
        }
        }

        return nullptr;
    }

    std::unique_ptr<physics::Object> SceneLoader::createPhysicsObject(
        const EntityCloneData& entity,
        const PhysicsData& data,
        const int cloneIndex,
        const glm::uvec3& tile)
    {
        //switch (data.type) {
        //case CustomMaterialType::text: {
        //    auto material{ std::make_unique<TextMaterial>() };
        //    material->m_fontName = data.fontName;
        //    material->m_fontSize = data.fontSize;

        //    return material;
        //}
        //}

        return nullptr;
    }

    void SceneLoader::loadMeta(
        const YAML::Node& doc,
        MetaData& data)
    {
        data.name = "<noname>";
        data.modelsDir = m_assets.modelsDir;

        auto& node = doc["meta"];

        if (!node) return;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "name") {
                data.name= readString(v);
            }
            else if (k == "assetsDir") {
                data.assetsDir = readString(v);
            }
            else if (k == "modelsDir") {
                data.modelsDir = readString(v);
            }
            else {
                reportUnknown("meta_entry", k, v);
            }
        }
    }

    void SceneLoader::loadSkybox(
        const YAML::Node& doc,
        SkyboxData& data)
    {
        auto& node = doc["skybox"];

        if (!node) return;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "program" || k == "shader") {
                data.programName = readString(v);
                data.programName = "skybox";
            }
            else if (k == "material") {
                data.materialName = readString(v);
            }
            else if (k == "priority") {
                data.priority = readInt(v);
            }
            else if (k == "gamma_correct") {
                data.gammaCorrect = readBool(v);
            }
            else if (k == "hdri") {
                data.hdri = readBool(v);
            }
            else if (k == "swap_faces") {
                data.swapFaces = readBool(v);
            }
            else if (k == "faces") {
                loadSkyboxFaces(v, data);
            }
            else {
                reportUnknown("skybox_entry", k, v);
            }
        }

        if (util::matchAny(hdriMatchers, data.materialName)) {
            data.hdri = true;
        }

        if (data.hdri) {
            data.gammaCorrect = false;
        }
    }

    void SceneLoader::loadSkyboxFaces(
        const YAML::Node& node,
        SkyboxData& data)
    {
        if (!node.IsSequence()) {
            return;
        }

        int idx = 0;
        for (const auto& e : node) {
            data.faces[idx] = e.as<std::string>();
            idx++;
        }

        data.loadedFaces = true;
    }

    void SceneLoader::loadRoot(
        const YAML::Node& doc,
        EntityData& root)
    {
        auto& node = doc["root"];
        loadEntity(node, root);

        root.base.idBase = { ROOT_UUID };
        root.base.type = EntityType::origo;
        root.isRoot = true;
        root.base.enabled = true;
    }

    void SceneLoader::loadEntities(
        const YAML::Node& doc,
        std::vector<EntityData>& entities)
    {
        for (const auto& entry : doc["entities"]) {
            auto& data = entities.emplace_back();
            loadEntity(entry, data);
        }
    }

    void SceneLoader::loadEntity(
        const YAML::Node& node,
        EntityData& data)
    {
        loadEntityClone(node, data.base, data.clones, true);
    }

    void SceneLoader::loadEntityClone(
        const YAML::Node& node,
        EntityCloneData& data,
        std::vector<EntityCloneData>& clones,
        bool recurse)
    {
        bool hasClones = false;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "origo") {
                    data.type = EntityType::origo;
                }
                else if (type == "container") {
                    data.type = EntityType::container;
                }
                else if (type == "model") {
                    data.type = EntityType::model;
                }
                else if (type == "quad") {
                    data.type = EntityType::quad;
                }
                else if (type == "billboard") {
                    data.type = EntityType::billboard;
                }
                else if (type == "sprite") {
                    data.type = EntityType::sprite;
                }
                else if (type == "terrain") {
                    data.type = EntityType::terrain;
                }
                else {
                    reportUnknown("entity_type", k, v);
                }
            }
            else if (k == "priority") {
                data.priority = readInt(v);
            }
            else if (k == "name") {
                data.name = readString(v);
            }
            else if (k == "desc") {
                data.desc = readString(v);
            }
            else if (k == "id") {
                data.idBase = readUUID(v);
            }
            else if (k == "parent_id") {
                data.parentIdBase = readUUID(v);
            }
            else if (k == "model") {
                if (v.Type() == YAML::NodeType::Sequence) {
                    data.meshPath = v[0].as<std::string>();
                    data.meshName = v[1].as<std::string>();
                }
                else {
                    data.meshName = readString(v);
                }
            }
            else if (k == "program" || k == "shader") {
                data.programName = readString(v);
                if (data.programName == "texture") {
                    data.programName = SHADER_TEXTURE;
                }
            }
            else if (k == "depth_program") {
                data.depthProgramName = readString(v);
            }
            else if (k == "geometry_type") {
                data.geometryType = readString(v);
            }
            else if (k == "program_definitions" || k == "shader_definitions") {
                for (const auto& defNode : v) {
                    auto defName = defNode.first.as<std::string>();
                    const auto& defValue = defNode.second.as<std::string>();
                    data.programDefinitions[util::toUpper(defName)] = defValue;
                }
            }
            else if (k == "render_flags") {
                for (const auto& flagNode : v) {
                    auto flagName = flagNode.first.as<std::string>();
                    const auto flagValue = readBool(flagNode.second);
                    data.renderFlags[util::toLower(flagName)] = flagValue;
                }
            }
            else if (k == "front") {
                data.front = readVec3(v);
            }
            else if (k == "material") {
                data.materialName = readString(v);
            }
            else if (k == "material_modifier") {
                loadMaterialModifiers(v, data);
            }
            else if (k == "force_material") {
                data.forceMaterial = readBool(v);
            }
            else if (k == "sprite") {
                data.spriteName = readString(v);
            }
            else if (k == "batch_size") {
                data.batchSize = readInt(v);
            }
            else if (k == "load_textures") {
                data.loadTextures = readBool(v);
            }
            else if (k == "position" || k == "pos") {
                data.position = readVec3(v);
            }
            else if (k == "rotation" || k == "rot") {
                data.rotation = readVec3(v);
            }
            else if (k == "scale") {
                data.scale = readScale3(v);
            }
            else if (k == "repeat") {
                loadRepeat(v, data.repeat);
            }
            else if (k == "tiling") {
                loadTiling(v, data.tiling);
            }
            else if (k == "camera") {
                m_cameraLoader.loadCamera(v, data.camera);
            }
            else if (k == "light") {
                m_lightLoader.loadLight(v, data.light);
            }
            else if (k == "custom_material") {
                loadCustomMaterial(v, data.customMaterial);
            }
            else if (k == "physics") {
                loadPhysics(v, data.physics);
            }
            else if (k == "controller") {
                m_controllerLoader.loadController(v, data.controller);
            }
            else if (k == "generator") {
                m_generatorLoader.loadGenerator(v, data.generator);
            }
            else if (k == "instanced") {
                data.instanced = readBool(v);
            }
            else if (k == "selected") {
                data.selected = readBool(v);
            }
            else if (k == "enabled") {
                data.enabled = readBool(v);
            }
            else if (k == "clone_position_offset") {
                data.clonePositionOffset = readVec3(v);
            }
            else if (k == "clone_mesh") {
                data.cloneMesh = readBool(v);
            }
            else if (k == "tile") {
                data.tile = readVec3(v);
            }
            else if (k == "clones") {
                if (recurse)
                    hasClones = true;
            }
            else if (k == "script") {
                data.script = readString(v);
            }
            else if (k == "script_file") {
                std::string filename = readString(v) + ".lua";
                data.script = readFile(filename);
            }
            else {
                reportUnknown("entity_entry", k, v);
            }
        }

        if (hasClones) {
            for (const auto& pair : node) {
                const std::string& k = pair.first.as<std::string>();
                const YAML::Node& v = pair.second;

                if (k == "clones") {
                    for (const auto& node : v) {
                        // NOTE KI intialize with current data
                        EntityCloneData clone = data;
                        std::vector<EntityCloneData> dummy{};
                        loadEntityClone(node, clone, dummy, false);
                        clones.push_back(clone);
                    }
                }
            }
        }
    }

    void SceneLoader::loadMaterialModifiers(
        const YAML::Node& node,
        EntityCloneData& data)
    {
        data.materialModifiers.enabled = true;
        data.materialModifiers.material.m_name = "<modifier>";

        m_materialLoader.loadMaterial(node, data.materialModifiers);
    }

    void SceneLoader::loadCustomMaterial(
        const YAML::Node& node,
        CustomMaterialData& data)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = CustomMaterialType::none;
                }
                else if (type == "skybox") {
                    data.type = CustomMaterialType::skybox;
                }
                else if (type == "text") {
                    data.type = CustomMaterialType::text;
                }
                else {
                    reportUnknown("custom_material_type", k, v);
                }
            }
            else if (k == "font") {
                data.fontName = readString(v);
            }
            else if (k == "font_size") {
                data.fontSize = readFloat(v);
            }
            else {
                reportUnknown("custom_material_entry", k, v);
            }
        }
    }

    void SceneLoader::loadPhysics(
        const YAML::Node& node,
        PhysicsData& data)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            // NOTE KI physics needs body or gem
            if (k == "space") {
                data.space = readString(v);
            }
            else if (k == "body") {
                data.enabled = true;
                loadBody(v, data.body);
            }
            else if (k == "geom") {
                data.enabled = true;
                loadGeom(v, data.geom);
            }
            else {
                reportUnknown("physics_entry", k, v);
            }
        }
    }

    void SceneLoader::loadBody(
        const YAML::Node& node,
        BodyData& data)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = BodyType::none;
                }
                else if (type == "sphere") {
                    data.type = BodyType::sphere;
                }
                else if (type == "box") {
                    data.type = BodyType::box;
                }
                else {
                    reportUnknown("body_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "mass") {
                data.mass = readFloat(v);
            }
            else if (k == "linear_vel") {
                data.linearVel = readVec3(v);
            }
            else if (k == "angular_vel") {
                data.angularVel = readVec3(v);
            }
            else if (k == "rotation") {
                data.rotation = readVec3(v);
            }
            else {
                reportUnknown("body_entry", k, v);
            }
        }
    }

    void SceneLoader::loadGeom(
        const YAML::Node& node,
        GeomData& data)
    {
        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;

            if (k == "type") {
                std::string type = readString(v);
                if (type == "none") {
                    data.type = GeomType::none;
                }
                else if (type == "plane") {
                    data.type = GeomType::plane;
                }
                else if (type == "sphere") {
                    data.type = GeomType::sphere;
                }
                else if (type == "box") {
                    data.type = GeomType::box;
                }
                else if (type == "capsule") {
                    data.type = GeomType::capsule;
                }
                else if (type == "cylinder") {
                    data.type = GeomType::cylinder;
                }
                else {
                    reportUnknown("geom_type", k, v);
                }
            }
            else if (k == "size") {
                data.size = readVec3(v);
            }
            else if (k == "plane") {
                data.plane = readVec4(v);
            }
            else if (k == "category") {
                data.category = readInt(v);
            }
            else if (k == "collide") {
                data.collide = readInt(v);
            }
            else {
                reportUnknown("geom_entry", k, v);
            }
        }
    }

    void SceneLoader::loadSprites(
        const YAML::Node& doc,
        std::vector<Sprite>& sprites)
    {
        for (const auto& entry : doc["sprites"]) {
            Sprite& sprite = sprites.emplace_back();
            loadSprite(entry, sprite);
        }
    }

    void SceneLoader::loadSprite(
        const YAML::Node& node,
        Sprite& sprite)
    {
        for (const auto& pair : node) {
            auto key = pair.first.as<std::string>();
            const std::string k = util::toLower(key);
            const YAML::Node& v = pair.second;

            if (k == "name") {
                sprite.m_name = readString(v);
            }
            else if (k == "shapes") {
                loadShapes(v, sprite.m_shapes);
            }
        }
    }

    void SceneLoader::loadShapes(
        const YAML::Node& node,
        std::vector<Shape>& shapes)
    {
        for (const auto& entry : node) {
            Shape& shape = shapes.emplace_back();
            loadShape(entry, shape);
        }
    }

    void SceneLoader::loadShape(
        const YAML::Node& node,
        Shape& shape)
    {
        for (const auto& pair : node) {
            auto key = pair.first.as<std::string>();
            const YAML::Node& v = pair.second;
            const std::string k = util::toLower(key);

            if (k == "rotation") {
                shape.m_rotation = readFloat(v);
            }
            //else if (k == "material") {
            //    loadMaterial(
            //        v,
            //        shape.m_materialFields,
            //        shape.m_material);
            //}
            else {
                reportUnknown("shape_entry", k, v);
            }
        }
    }
}
