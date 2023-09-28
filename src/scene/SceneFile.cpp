#include "SceneFile.h"

#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <filesystem>
#include <regex>

#include <fmt/format.h>

#include "util/Util.h"

#include "asset/Material.h"
#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"
#include "asset/SpriteMesh.h"
#include "asset/Sprite.h"
#include "asset/Shape.h"
#include "asset/Program.h"
#include "asset/Shader.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "model/Node.h"

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
    const float DEF_ALPHA = 1.0;

    const std::string AUTO_UUID{ "AUTO" };
    const std::string ROOT_UUID{ "ROOT" };
    const std::string VOLUME_UUID{ "VOLUME" };
    const std::string CUBE_MAP_UUID{ "CUBE_MAP" };

    const std::string MACRO_STEP_X{ "X" };
    const std::string MACRO_STEP_Y{ "Y" };
    const std::string MACRO_STEP_Z{ "Z" };

    const std::string QUAD_MESH_NAME{ "quad" };
    const std::string SKYBOX_MESH_NAME{ "quad_skybox" };

    std::mutex uuid_lock{};

    const std::vector<std::regex> texturesMatchers{
        std::regex("textures"),
    };

    const std::vector<std::regex> hdriMatchers{
        std::regex(".*[\\.]hdr"),
    };

    const std::vector<std::regex> ignoreMatchers{
        std::regex(".*nope.*"),
        std::regex(".*[\\.]blend"),
        std::regex(".*[\\.]exr"),
        std::regex(".*[\\.]txt"),
        std::regex(".*[\\.]usda"),
        std::regex(".*preview.*"),
    };

    const std::vector<std::regex> validMatchers{
        std::regex(".*[\\.]hdr"),
        std::regex(".*[\\.]png"),
        std::regex(".*[\\.]jpg"),
    };

    const std::vector<std::regex> colorMatchers{
        std::regex(".*[-_ ]color[-_ \\.].*"),
        std::regex(".*[-_ ]col[-_ \\.].*"),
        std::regex(".*[-_ ]basecolor[-_ \\.].*"),
        std::regex(".*[-_ ]diff[-_ \\.].*"),
    };

    const std::vector<std::regex> normalMatchers{
        std::regex(".*[-_ ]normal[-_ \\.].*"),
        std::regex(".*[-_ ]normalgl[-_ \\.].*"),
        std::regex(".*[-_ ]nrm[-_ \\.].*"),
        std::regex(".*[-_ ]nor[-_ \\.].*"),
    };

    const std::vector<std::regex> metalnessMatchers{
        std::regex(".*[-_ ]metalness[-_ \\.].*"),
        std::regex(".*[-_ ]met[-_ \\.].*"),
        std::regex(".*[-_ ]metallic[-_ \\.].*"),
    };

    const std::vector<std::regex> roughnessMatchers{
        std::regex(".*[-_ ]roughness[-_ \\.].*"),
        std::regex(".*[-_ ]rough[-_ \\.].*"),
        std::regex(".*[-_ ]rgh[-_ \\.].*"),
    };

    const std::vector<std::regex> occlusionMatchers{
        std::regex(".*[-_ ]ambientocclusion[-_ \\.].*"),
        std::regex(".*[-_ ]occlusion[-_ \\.].*"),
        std::regex(".*[-_ ]ao[-_ \\.].*"),
    };

    const std::vector<std::regex> displacementMatchers{
        std::regex(".*[-_ ]displacement[-_ \\.].*"),
        std::regex(".*[-_ ]disp[-_ \\.].*"),
    };

    const std::vector<std::regex> opacityMatchers{
        std::regex(".*[-_ ]opacity[-_ \\.].*"),
        std::regex(".*[-_ ]ops[-_ \\.].*"),
    };
}

template <> struct fmt::formatter<SceneFile::BaseUUID> {
    // Parses format specifications of the form ['f' | 'e'].
    constexpr auto parse(fmt::format_parse_context& ctx) -> decltype(ctx.begin()) {
        auto it = ctx.begin();
        return it;
    }

    template <typename FormatContext>
    auto format(const SceneFile::BaseUUID& p, FormatContext& ctx) const -> decltype(ctx.out()) {
        if (p.empty()) {
            return ctx.out();
        }
        else if (p.size() == 1) {
            return fmt::format_to(
                ctx.out(),
                "{}", p[0]);
        } else {
            return fmt::format_to(
                ctx.out(),
                "{}-{}", p[0], p[1]);
        }
    }
};

SceneFile::SceneFile(
    const Assets& assets,
    std::shared_ptr<std::atomic<bool>> alive,
    std::shared_ptr<AsyncLoader> asyncLoader,
    const std::string& filename)
    : m_filename(filename),
    m_dirname(util::dirName(filename)),
    m_alive(alive),
    m_assets(assets),
    m_asyncLoader(asyncLoader)
{
}

SceneFile::~SceneFile()
{
    KI_INFO(fmt::format("SCENE_FILE: delete - file={}", m_filename));

    m_defaultMaterial = Material::createMaterial(BasicMaterial::basic);
}

void SceneFile::load(
    std::shared_ptr<Registry> registry)
{
    m_registry = registry;
    m_dispatcher = registry->m_dispatcher;

    if (!util::fileExists(m_filename)) {
        throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", m_filename) };
    }

    m_asyncLoader->addLoader(m_alive, [this]() {
        std::ifstream fin(m_filename);
        YAML::Node doc = YAML::Load(fin);

        loadMeta(doc, m_meta);

        loadSkybox(doc, m_skybox);
        loadMaterials(doc, m_materials);
        loadSprites(doc, m_sprites);

        loadRoot(doc, m_root);
        loadEntities(doc, m_entities);

        attach(m_skybox, m_root, m_entities, m_materials, m_sprites);
    });
}

void SceneFile::attach(
    SkyboxData& skybox,
    const EntityData& root,
    const std::vector<EntityData>& entities,
    std::vector<Material>& materials,
    std::vector<Sprite>& sprites)
{
    attachSkybox(root, skybox, materials);

    attachEntity(root, root, materials, sprites);
    attachVolume(root);
    attachCubeMapCenter(root);

    for (const auto& entity : entities) {
        attachEntity(root, entity, materials, sprites);
    }
}

void SceneFile::attachSkybox(
    const EntityData& root,
    SkyboxData& data,
    std::vector<Material>& materials)
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

void SceneFile::attachVolume(
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

void SceneFile::attachCubeMapCenter(
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

void SceneFile::attachEntity(
    const EntityData& root,
    const EntityData& data,
    std::vector<Material>& materials,
    std::vector<Sprite>& sprites)
{
    if (!data.base.enabled) {
        return;
    }

    m_asyncLoader->addLoader(m_alive, [this, &root, &data, &materials, &sprites]() {
        if (data.clones.empty()) {
            MeshType* type{ nullptr };
            attachEntityClone(type, root, data, data.base, false, 0, materials, sprites);
        }
        else {
            MeshType* type{ nullptr };

            int cloneIndex = 0;
            for (auto& cloneData : data.clones) {
                if (!*m_alive) return;
                type = attachEntityClone(type, root, data, cloneData, true, cloneIndex, materials, sprites);
                if (!data.base.cloneMesh)
                    type = nullptr;
                cloneIndex++;
            }
        }
    });
}

MeshType* SceneFile::attachEntityClone(
    MeshType* type,
    const EntityData& root,
    const EntityData& entity,
    const EntityCloneData& data,
    bool cloned,
    int cloneIndex,
    std::vector<Material>& materials,
    std::vector<Sprite>& sprites)
{
    if (!*m_alive) return type;

    if (!data.enabled) {
        return type;
    }

    const auto& repeat = data.repeat;

    for (auto z = 0; z < repeat.zCount; z++) {
        for (auto y = 0; y < repeat.yCount; y++) {
            for (auto x = 0; x < repeat.xCount; x++) {
                if (!*m_alive) return type;

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
                    materials,
                    sprites);

                if (!entity.base.cloneMesh)
                    type = nullptr;
            }
        }
    }

    return type;
}

MeshType* SceneFile::attachEntityCloneRepeat(
    MeshType* type,
    const EntityData& root,
    const EntityData& entity,
    const EntityCloneData& data,
    bool cloned,
    int cloneIndex,
    const glm::uvec3& tile,
    const glm::vec3& tilePositionOffset,
    std::vector<Material>& materials,
    std::vector<Sprite>& sprites)
{
    if (!*m_alive) return type;

    if (!data.enabled) {
        return type;
    }

    // NOTE KI overriding material in clones is *NOT* supported"
    if (!type) {
        type = createType(
            entity.isRoot,
            data,
            tile,
            materials,
            sprites);
        if (!type) return type;
    }

    if (!*m_alive) return type;

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
        auto* controller = createController(data, data.controller, node);

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

MeshType* SceneFile::createType(
    bool isRoot,
    const EntityCloneData& data,
    const glm::uvec3& tile,
    std::vector<Material>& materials,
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
        resolveMaterial(type, data, materials);
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
            if (data.materialModifiers_enabled) {
                modifyMaterial(m, data.materialModifierFields, data.materialModifiers);
            }
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

            if (type->m_flags.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
            }
            if (type->m_flags.blend) {
                definitions[DEF_USE_BLEND] = "1";
            }
            if (type->m_flags.blendOIT) {
                definitions[DEF_USE_BLEND_OIT] = "1";
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
        }
    }

    return type;
}

void SceneFile::resolveMaterial(
    MeshType* type,
    const EntityCloneData& data,
    std::vector<Material>& materials)
{
    // NOTE KI need to create copy *IF* modifiers
    // TODO KI should make copy *ALWAYS* for safety
    Material* material = nullptr;

    if (!data.materialName.empty()) {
        material = Material::find(data.materialName, materials);
    }

    if (!material) {
        material = &m_defaultMaterial;
    }

    auto& materialVBO = type->m_materialVBO;

    materialVBO.m_defaultMaterial = *material;
    materialVBO.m_useDefaultMaterial = true;
    materialVBO.m_forceDefaultMaterial = data.forceMaterial;
}

void SceneFile::resolveSprite(
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

void SceneFile::resolveMesh(
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

Node* SceneFile::createNode(
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

    if (data.camera.enabled) {
        node->m_camera = createCamera(data, data.camera);
    }

    if (data.light.enabled) {
        node->m_light = createLight(data, data.light, cloneIndex, tile);
    }

    if (data.generator.enabled) {
        node->m_generator = createGenerator(data, data.generator, node);
    }

    return node;
}

void SceneFile::assignFlags(
    const EntityCloneData& data,
    MeshType* type)
{
    NodeRenderFlags& flags = type->m_flags;

    flags.gbuffer = data.programName.starts_with("g_");

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

void SceneFile::modifyMaterial(
    Material& m,
    const MaterialField& f,
    const Material& mod)
{
    if (f.type) m.m_type = mod.m_type;

    if (f.textureSpec) m.textureSpec = mod.textureSpec;

    if (f.pattern) m.pattern = mod.pattern;
    if (f.reflection) m.reflection = mod.reflection;
    if (f.refraction) m.refraction = mod.refraction;
    if (f.refractionRatio) m.refractionRatio = mod.refractionRatio;

    if (f.tilingX) m.tilingX = mod.tilingX;
    if (f.tilingY) m.tilingY = mod.tilingY;

    if (f.ns) m.ns = mod.ns;

    if (f.ka) m.ka = mod.ka;

    if (f.kd) m.kd = mod.kd;
    if (f.map_kd) m.map_kd = mod.map_kd;

    if (f.ks) m.ks = mod.ks;
    if (f.map_ks) m.map_ks = mod.map_ks;
    if (f.ke) m.ke = mod.ke;
    if (f.map_ke) m.map_ke = mod.map_ke;
    if (f.map_bump) m.map_bump = mod.map_bump;
    if (f.map_bump_strength) m.map_bump_strength = mod.map_bump_strength;
    if (f.ni) m.ni = mod.ni;
    if (f.d) m.d = mod.d;
    if (f.illum) m.illum = mod.illum;

    if (f.layers) m.layers = mod.layers;
    if (f.layersDepth) m.layersDepth = mod.layersDepth;
    if (f.parallaxDepth) m.parallaxDepth = mod.parallaxDepth;

    if (f.map_dudv) m.map_dudv = mod. map_dudv;
    if (f.map_height) m.map_height = mod.map_height;
    if (f.map_noise) m.map_noise = mod.map_noise;

    if (f.map_roughness) m.map_roughness = mod.map_roughness;
    if (f.map_metalness) m.map_metalness = mod.map_metalness;
    if (f.map_occlusion) m.map_occlusion = mod.map_occlusion;
    if (f.map_displacement) m.map_displacement = mod.map_displacement;
    if (f.map_opacity) m.map_opacity = mod.map_opacity;
}

std::unique_ptr<Camera> SceneFile::createCamera(
    const EntityCloneData& entity,
    const CameraData& data)
{
    if (!data.enabled) return std::unique_ptr<Camera>();

    // NOTE only node cameras in scenefile for now
    auto camera = std::make_unique<Camera>();

    if (data.orthagonal) {
        camera->setViewport(data.viewport);
    }
    camera->setPosition(data.pos);
    camera->setAxis(data.front, data.up);
    camera->setRotation(data.rotation);
    camera->setFov(data.fov);

    camera->setEnabled(data.enabled);
    camera->setDefault(data.isDefault);

    return camera;
}

std::unique_ptr<Light> SceneFile::createLight(
    const EntityCloneData& entity,
    const LightData& data,
    const int cloneIndex,
    const glm::uvec3& tile)
{
    if (!data.enabled) return std::unique_ptr<Light>();

    auto light = std::make_unique<Light>();

    light->m_enabled = true;
    light->setPosition(data.pos);
    light->setTargetId(resolveUUID(data.targetIdBase, cloneIndex, tile));

    light->linear = data.linear;
    light->quadratic = data.quadratic;

    light->cutoffAngle = data.cutoffAngle;
    light->outerCutoffAngle = data.outerCutoffAngle;

    light->diffuse = data.diffuse;
    light->intensity = data.intensity;

    switch (data.type) {
    case LightType::directional:
        light->m_directional = true;
        break;
    case LightType::point:
        light->m_point = true;
        break;
    case LightType::spot:
        light->m_spot = true;
        break;
    }

    return light;
}

NodeController* SceneFile::createController(
    const EntityCloneData& entity,
    const ControllerData& data,
    Node* node)
{
    if (!data.enabled) return nullptr;

    const auto& center = node->getPosition();

    switch (data.type) {
        case ControllerType::camera: {
            auto* controller = new CameraController();
            return controller;
        }
    }

    return nullptr;
}

std::unique_ptr<NodeGenerator> SceneFile::createGenerator(
    const EntityCloneData& entity,
    const GeneratorData& data,
    Node* node)
{
    if (!data.enabled) return nullptr;

    const auto& center = node->getPosition();

    switch (data.type) {
    case GeneratorType::terrain: {
        auto generator = std::make_unique<TerrainGenerator>();

        auto& materialVBO = node->m_type->m_materialVBO;
        const auto& tiling = data.tiling;

        generator->m_modelsDir = m_meta.modelsDir;
        generator->m_worldTileSize = tiling.tile_size;
        generator->m_worldTilesU = tiling.tiles.x;
        generator->m_worldTilesV = tiling.tiles.z;
        generator->m_verticalRange = tiling.vertical_range;
        generator->m_horizontalScale = tiling.horizontal_scale;
        generator->m_material = materialVBO.m_defaultMaterial;

        return generator;
    }
    case GeneratorType::asteroid_belt: {
        auto generator = std::make_unique<AsteroidBeltGenerator>(data.count);
        return generator;
    }
    case GeneratorType::grid: {
        auto generator = std::make_unique<GridGenerator>();
        generator->m_xCount = data.repeat.xCount;
        generator->m_yCount = data.repeat.yCount;
        generator->m_zCount = data.repeat.zCount;

        generator->m_xStep = data.repeat.xStep;
        generator->m_yStep = data.repeat.yStep;
        generator->m_zStep = data.repeat.zStep;

        return generator;
    }
    }

    return nullptr;
}

void SceneFile::loadMeta(
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
            data.name= v.as<std::string>();
        }
        else if (k == "assetsDir") {
            data.assetsDir = v.as<std::string>();
        }
        else if (k == "modelsDir") {
            data.modelsDir = v.as<std::string>();
        }
        else {
            reportUnknown("meta_entry", k, v);
        }
    }
}

void SceneFile::loadSkybox(
    const YAML::Node& doc,
    SkyboxData& data)
{
    auto& node = doc["skybox"];

    if (!node) return;

    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "program" || k == "shader") {
            data.programName = v.as<std::string>();
            data.programName = "skybox";
        }
        else if (k == "material") {
            data.materialName = v.as<std::string>();
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

void SceneFile::loadSkyboxFaces(
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

void SceneFile::loadRoot(
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

void SceneFile::loadEntities(
    const YAML::Node& doc,
    std::vector<EntityData>& entities)
{
    for (const auto& entry : doc["entities"]) {
        auto& data = entities.emplace_back();
        loadEntity(entry, data);
    }
}

void SceneFile::loadEntity(
    const YAML::Node& node,
    EntityData& data)
{
    loadEntityClone(node, data.base, data.clones, true);
}

void SceneFile::loadEntityClone(
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
            std::string type = v.as<std::string>();
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
            data.name = v.as<std::string>();
        }
        else if (k == "desc") {
            data.desc = v.as<std::string>();
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
                data.meshName = v.as<std::string>();
            }
        }
        else if (k == "program" || k == "shader") {
            data.programName = v.as<std::string>();
            if (data.programName == "texture") {
                data.programName = SHADER_TEXTURE;
            }
        }
        else if (k == "geometry_type") {
            data.geometryType = v.as<std::string>();
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
            data.materialName = v.as<std::string>();
        }
        else if (k == "material_modifier") {
            loadMaterialModifiers(v, data);
        }
        else if (k == "force_material") {
            data.forceMaterial = readBool(v);
        }
        else if (k == "sprite") {
            data.spriteName = v.as<std::string>();
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
            loadCamera(v, data.camera);
        }
        else if (k == "light") {
            loadLight(v, data.light);
        }
        else if (k == "controller") {
            loadController(v, data.controller);
        }
        else if (k == "generator") {
            loadGenerator(v, data.generator);
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
            data.script = v.as<std::string>();
        }
        else if (k == "script_file") {
            std::string filename = v.as<std::string>() + ".lua";
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

void SceneFile::loadMaterialModifiers(
    const YAML::Node& node,
    EntityCloneData& data)
{
    data.materialModifiers_enabled = true;
    data.materialModifiers.m_name = "<modifier>";

    loadMaterial(node, data.materialModifierFields, data.materialModifiers);
}

void SceneFile::loadRepeat(
    const YAML::Node& node,
    Repeat& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "x_count") {
            data.xCount = readInt(v);
        }
        else if (k == "y_count") {
            data.yCount = readInt(v);
        }
        else if (k == "z_count") {
            data.zCount = readInt(v);
        }
        else if (k == "x_step") {
            data.xStep = readFloat(v);
        }
        else if (k == "y_step") {
            data.yStep = readFloat(v);
        }
        else if (k == "z_step") {
            data.zStep = readFloat(v);
        }
        else {
            reportUnknown("repeat_entry", k, v);
        }
    }
}

void SceneFile::loadTiling(
    const YAML::Node& node,
    Tiling& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "tiles") {
            data.tiles = readUVec3(v);
        }
        else if (k == "tile_size") {
            data.tile_size = readInt(v);
        }
        else if (k == "height_scale") {
            data.height_scale = readFloat(v);
        }
        else if (k == "vertical_range" || k == "vert_range") {
            data.vertical_range = readVec2(v);
        }
        else if (k == "horizontal_scale" || k == "horiz_scale") {
            data.horizontal_scale = readFloat(v);
        }
        else {
            reportUnknown("tiling_entry", k, v);
        }
    }
}

void SceneFile::loadCamera(const YAML::Node& node, CameraData& data)
{
    data.fov = m_assets.cameraFov;

    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = readBool(v);
        }
        else if (k == "default") {
            data.isDefault = readBool(v);
        }
        else if (k == "fov") {
            data.fov = readFloat(v);
        }
        else if (k == "front") {
            data.front = readVec3(v);
        }
        else if (k == "up") {
            data.up = readVec3(v);
        }
        else if (k == "pos") {
            data.pos = readVec3(v);
        }
        else if (k == "rotation") {
            data.rotation = readVec3(v);
        }
        else if (k == "orthagonal") {
            data.orthagonal = readBool(v);
        }
        else if (k == "viewport") {
            const auto& vec = readVec4(v);
            data.viewport = { vec[0], vec[1], vec[2], vec[3] };
        }
        else {
            reportUnknown("controller_entry", k, v);
        }
    }
}

void SceneFile::loadLight(const YAML::Node& node, LightData& data)
{
    // Default to center
    data.targetIdBase = { ROOT_UUID };

    // pos relative to owning node
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = readBool(v);
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "none") {
                data.type = LightType::none;
            }
            else if (type == "directional") {
                data.type = LightType::directional;
            }
            else if (type == "point") {
                data.type = LightType::point;
            }
            else if (type == "spot") {
                data.type = LightType::spot;
            }
            else {
                reportUnknown("light_type", k, v);
            }
        }
        else if (k == "pos") {
            data.pos = readVec3(v);
        }
        else if (k == "target_id") {
            data.targetIdBase = readUUID(v);
        }
        else if (k == "linear") {
            data.linear = readFloat(v);
        }
        else if (k == "quadratic") {
            data.quadratic = readFloat(v);
        }
        else if (k == "cutoff_angle") {
            data.cutoffAngle = readFloat(v);
        }
        else if (k == "outer_cutoff_angle") {
            data.outerCutoffAngle = readFloat(v);
        }
        else if (k == "diffuse") {
            data.diffuse = readRGB(v);
        }
        else if (k == "intensity") {
            data.intensity = readFloat(v);
        }
        else {
            reportUnknown("light_entry", k, v);
        }
    }
}

void SceneFile::loadController(const YAML::Node& node, ControllerData& data)
{
    // pos relative to owning node
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = readBool(v);
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "none") {
                data.type = ControllerType::none;
            }
            else if (type == "camera") {
                data.type = ControllerType::camera;
            }
            else {
                reportUnknown("controller_type", k, v);
            }
        }
        else if (k == "speed") {
            data.speed = readFloat(v);
        }
        else if (k == "mode") {
            data.mode = readInt(v);
        }
        else {
            reportUnknown("controller_entry", k, v);
        }
    }
}

void SceneFile::loadGenerator(
    const YAML::Node& node,
    GeneratorData& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = readBool(v);
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "none") {
                data.type = GeneratorType::none;
            }
            else if (type == "terrain") {
                data.type = GeneratorType::terrain;
            }
            else if (type == "grid") {
                data.type = GeneratorType::grid;
            }
            else if (type == "asteroid_belt") {
                data.type = GeneratorType::asteroid_belt;
            }
            else {
                reportUnknown("generator_type", k, v);
            }
        }
        else if (k == "count") {
            data.count = readInt(v);
        }
        else if (k == "radius") {
            data.radius = readFloat(v);
        }
        else if (k == "mode") {
            data.mode = readInt(v);
        }
        else if (k == "repeat") {
            loadRepeat(v, data.repeat);
        }
        else if (k == "tiling") {
            loadTiling(v, data.tiling);
        }
        else {
            reportUnknown("generator_entry", k, v);
        }
    }
}

void SceneFile::loadMaterials(
    const YAML::Node& doc,
    std::vector<Material>& materials) {
    for (const auto& entry : doc["materials"]) {

        MaterialField fields;
        Material& material = materials.emplace_back();
        loadMaterial(entry, fields, material);
    }
}

void SceneFile::loadMaterial(
    const YAML::Node& node,
    MaterialField& fields,
    Material& material)
{
    for (const auto& pair : node) {
        auto key = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;
        const std::string k = util::toLower(key);

        if (k == "name") {
            material.m_name = v.as<std::string>();
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "model") {
                material.m_type = MaterialType::model;
                fields.type = true;
            }
            else if (type == "texture") {
                material.m_type = MaterialType::texture;
                fields.type = true;
            }
            else if (type == "sprite") {
                material.m_type = MaterialType::sprite;
                fields.type = true;
            }
            else {
                reportUnknown("material_type", k, v);
            }
        }
        else if (k == "ns") {
            material.ns = readFloat(v);
            fields.ns = true;
        }
        else if (k == "ka") {
            material.ka = readRGB(v);
            fields.ka = true;
        }
        else if (k == "kd") {
            material.kd = readRGBA(v);
            fields.kd = true;
        }
        else if (k == "ks") {
            material.ks = readRGB(v);
            fields.ks = true;
        }
        else if (k == "ke") {
            material.ke = glm::vec4(readRGB(v), 1.f);
            fields.ke = true;
        }
        else if (k == "ni") {
            material.ni = readFloat(v);
            fields.ni = true;
        }
        else if (k == "d") {
            material.d = readFloat(v);
            fields.d = true;
        }
        else if (k == "illum") {
            material.d = readFloat(v);
            fields.illum = true;
        }
        else if (k == "map_pbr") {
            std::string line = v.as<std::string>();
            loadMaterialPbr(
                line,
                fields,
                material);
        }
        else if (k == "map_kd") {
            std::string line = v.as<std::string>();
            material.map_kd = resolveTexturePath(line);
            fields.map_kd = true;
        }
        else if (k == "map_ke") {
            std::string line = v.as<std::string>();
            material.map_ke = resolveTexturePath(line);
            fields.map_ke = true;
        }
        else if (k == "map_ks") {
            std::string line = v.as<std::string>();
            material.map_ks = resolveTexturePath(line);
            fields.map_ks = true;
        }
        else if (k == "map_bump" || k == "bump") {
            std::string line = v.as<std::string>();
            material.map_bump = resolveTexturePath(line);
            fields.map_bump = true;
        }
        else if (k == "map_bump_strength") {
            material.map_bump_strength = readFloat(v);
            fields.map_bump_strength = true;
        }
        else if (k == "map_dudv") {
            std::string line = v.as<std::string>();
            material.map_dudv = resolveTexturePath(line);
            fields.map_dudv = true;
        }
        else if (k == "map_height") {
            std::string line = v.as<std::string>();
            material.map_height = resolveTexturePath(line);
            fields.map_height = true;
        }
        else if (k == "map_noise") {
            std::string line = v.as<std::string>();
            material.map_noise = resolveTexturePath(line);
            fields.map_noise = true;
        }
        else if (k == "map_roughness") {
            std::string line = v.as<std::string>();
            material.map_roughness = resolveTexturePath(line);
            fields.map_roughness = true;
        }
        else if (k == "map_metalness") {
            std::string line = v.as<std::string>();
            material.map_metalness = resolveTexturePath(line);
            fields.map_metalness = true;
        }
        else if (k == "map_occlusion") {
            std::string line = v.as<std::string>();
            material.map_occlusion = resolveTexturePath(line);
            fields.map_occlusion = true;
        }
        else if (k == "map_displacement") {
            std::string line = v.as<std::string>();
            material.map_displacement = resolveTexturePath(line);
            fields.map_displacement = true;
        }
        else if (k == "map_opacity") {
            std::string line = v.as<std::string>();
            material.map_opacity = resolveTexturePath(line);
            fields.map_opacity = true;
        }
        else if (k == "metal") {
            material.metal = readVec4(v);
            fields.metal = true;
        }
        else if (k == "pattern") {
            material.pattern = readInt(v);
            fields.pattern = true;
        }
        else if (k == "reflection") {
            material.reflection = readFloat(v);
            fields.reflection = true;
        }
        else if (k == "refraction") {
            material.refraction = readFloat(v);
            fields.refraction = true;
        }
        else if (k == "refraction_ratio") {
            material.refractionRatio = readRefractionRatio(v);
            fields.refractionRatio = true;
        }
        else if (k == "tiling") {
            material.tilingX = readFloat(v);
            material.tilingY = readFloat(v);
            fields.tilingX = true;
            fields.tilingY = true;
        }
        else if (k == "tiling_x") {
            material.tilingX = readFloat(v);
            fields.tilingX = true;
        }
        else if (k == "tiling_y") {
            material.tilingY = readFloat(v);
            fields.tilingY = true;
        }
        else if (k == "layers") {
            material.layers = readInt(v);
            fields.layers = true;
        }
        else if (k == "layers_depth") {
            material.layersDepth = readFloat(v);
            fields.layersDepth = true;
        }
        else if (k == "parallax_depth") {
            material.parallaxDepth = readFloat(v);
            fields.parallaxDepth = true;
        }
        else if (k == "texture_spec") {
            loadTextureSpec(v, material.textureSpec);
            fields.textureSpec = true;
        }
        else {
            reportUnknown("material_entry", k, v);
        }
    }
}

void SceneFile::loadMaterialPbr(
    const std::string& pbrName,
    MaterialField& fields,
    Material& material)
{
    const std::string basePath = util::joinPath(
        m_assets.assetsDir,
        pbrName);

    for (const auto& dirEntry : std::filesystem::directory_iterator(basePath)) {
        std::string fileName = dirEntry.path().filename().string();
        std::string assetPath = util::joinPath(pbrName,fileName);

        std::string matchName{ util::toLower(fileName) };

        if (util::matchAny(texturesMatchers, matchName)) {
            loadMaterialPbr(pbrName + "\\" + fileName, fields, material);
            return;
        }

        if (util::matchAny(ignoreMatchers, matchName)) {
            continue;
        }

        if (!util::matchAny(validMatchers, matchName)) {
            continue;
        }

        bool found = false;

        if (!found && util::matchAny(colorMatchers, matchName)) {
            fields.map_kd = true;
            material.map_kd = assetPath;
            found = true;
        }

        if (!found && util::matchAny(normalMatchers, matchName)) {
            fields.map_bump = true;
            material.map_bump = assetPath;
            found = true;
        }

        if (!found && util::matchAny(metalnessMatchers, matchName)) {
            fields.map_metalness = true;
            material.map_metalness = assetPath;
            found = true;
        }

        if (!found && util::matchAny(roughnessMatchers, matchName)) {
            fields.map_roughness = true;
            material.map_roughness = assetPath;
            found = true;
        }

        if (!found && util::matchAny(occlusionMatchers, matchName)) {
            fields.map_occlusion = true;
            material.map_occlusion = assetPath;
            found = true;
        }

        if (!found && util::matchAny(displacementMatchers, matchName)) {
            fields.map_displacement = true;
            material.map_displacement = assetPath;
            found = true;
        }

        if (!found && util::matchAny(opacityMatchers, matchName)) {
            fields.map_opacity = true;
            material.map_opacity = assetPath;
            found = true;
        }

        if (!found) {
            throw std::runtime_error{ fmt::format("UNKNOWN_PBR_FILE: {}", assetPath) };
        }
    }
}

void SceneFile::loadSprites(
    const YAML::Node& doc,
    std::vector<Sprite>& sprites)
{
    for (const auto& entry : doc["sprites"]) {
        Sprite& sprite = sprites.emplace_back();
        loadSprite(entry, sprite);
    }
}

void SceneFile::loadSprite(
    const YAML::Node& node,
    Sprite& sprite)
{
    for (const auto& pair : node) {
        auto key = pair.first.as<std::string>();
        const std::string k = util::toLower(key);
        const YAML::Node& v = pair.second;

        if (k == "name") {
            sprite.m_name = v.as<std::string>();
        }
        else if (k == "shapes") {
            loadShapes(v, sprite.m_shapes);
        }
    }
}

void SceneFile::loadShapes(
    const YAML::Node& node,
    std::vector<Shape>& shapes)
{
    for (const auto& entry : node) {
        Shape& shape = shapes.emplace_back();
        loadShape(entry, shape);
    }
}

void SceneFile::loadShape(
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
        else if (k == "material") {
            loadMaterial(
                v,
                shape.m_materialFields,
                shape.m_material);
        }
        else {
            reportUnknown("shape_entry", k, v);
        }
    }
}

void SceneFile::loadTextureSpec(
    const YAML::Node& node,
    TextureSpec& textureSpec)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "wrap") {
            loadTextureWrap(k, v, textureSpec.wrapS);
            loadTextureWrap(k, v, textureSpec.wrapT);
        }
        else if (k == "wrap_s") {
            loadTextureWrap(k, v, textureSpec.wrapS);
        }
        else if (k == "wrap_t") {
            loadTextureWrap(k, v, textureSpec.wrapT);
        }
        else {
            reportUnknown("tex_spec", k, v);
        }
    }
}

void SceneFile::loadTextureWrap(
    const std::string& k,
    const YAML::Node& v,
    GLint& wrapMode)
{
    const std::string& wrap = v.as<std::string>();
    if (wrap == "GL_REPEAT") {
        wrapMode = GL_REPEAT;
    }
    else if (wrap == "GL_CLAMP_TO_EDGE") {
        wrapMode = GL_CLAMP_TO_EDGE;
    }
    else {
        wrapMode = GL_CLAMP_TO_EDGE;
        reportUnknown("wrap_mode", k, v);
    }
}

bool SceneFile::readBool(const YAML::Node& node) const
{
    if (!util::isBool(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid bool={}", renderNode(node)));
        return false;
    }

    return node.as<bool>();
}

int SceneFile::readInt(const YAML::Node& node) const
{
    if (!util::isInt(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid int{}", renderNode(node)));
        return 0;
    }

    return node.as<int>();
}

float SceneFile::readFloat(const YAML::Node& node) const
{
    if (!util::isFloat(node.as<std::string>())) {
        KI_WARN(fmt::format("invalid float {}", renderNode(node)));
        return 0.f;
    }

    return node.as<float>();
}

std::vector<int> SceneFile::readIntVector(const YAML::Node& node, int reserve) const
{
    std::vector<int> a;
    a.reserve(reserve);

    for (const auto& e : node) {
        a.push_back(readInt(e));
    }

    return a;
}

std::vector<float> SceneFile::readFloatVector(const YAML::Node& node, int reserve) const
{
    std::vector<float> a;
    a.reserve(reserve);

    for (const auto& e : node) {
        a.push_back(readFloat(e));
    }

    return a;
}

glm::vec2 SceneFile::readVec2(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readFloatVector(node, 2);

        if (a.size() == 0) {
            a.push_back(0.f);
            a.push_back(0.f);
        }
        else if (a.size() == 1) {
            // FILL x, x
            a.push_back(a[0]);
        }

        return glm::vec2{ a[0], a[1] };
    }

    auto v = readFloat(node);
    return glm::vec2{ v };
}

glm::vec3 SceneFile::readVec3(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readFloatVector(node, 3);

        if (a.size() == 0) {
            a.push_back(0.f);
            a.push_back(0.f);
            a.push_back(0.f);
        }
        else if (a.size() == 1) {
            // FILL x, x, x
            a.push_back(a[0]);
            a.push_back(a[0]);
        }
        else if (a.size() == 2) {
            // FILL x, 0, z
            a.push_back(a[1]);
            a[1] = 0.f;
        }
        return glm::vec3{ a[0], a[1], a[2] };
    }

    auto v = readFloat(node);
    return glm::vec3{ v };
}

glm::vec4 SceneFile::readVec4(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readFloatVector(node, 4);

        if (a.size() == 0) {
            a.push_back(0.f);
            a.push_back(0.f);
            a.push_back(0.f);
            a.push_back(1.f);
        }
        else if (a.size() == 1) {
            a.push_back(a[0]);
            a.push_back(a[0]);
        }
        else if (a.size() == 2) {
            // FilL: x, 0, z, 1
            a.push_back(a[1]);
            a[1] = 0.f;
            // w == 1.f
            a.push_back(1.f);
        }
        else if (a.size() == 3) {
            // FILL x, y, z, 1
            // w == 1.f
            a.push_back(1.f);
        }

        return glm::vec4{ a[0], a[1], a[2], a[3] };
    }

    auto v = readFloat(node);
    return glm::vec4{ v };
}

glm::uvec3 SceneFile::readUVec3(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readIntVector(node, 3);

        if (a.size() == 0) {
            a.push_back(0);
            a.push_back(0);
            a.push_back(0);
        }
        else if (a.size() == 1) {
            // FILL x, x, x
            a.push_back(a[0]);
            a.push_back(a[0]);
        }
        else if (a.size() == 2) {
            // FILL x, 0, z
            a.push_back(a[1]);
            a[1] = 0;
        }

        return glm::uvec3{ a[0], a[1], a[2] };
    }

    auto v = node.as<unsigned int>();
    return glm::uvec3{ v };
}

glm::vec3 SceneFile::readScale3(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readFloatVector(node, 3);

        while (a.size() < 3) {
            a.push_back(1.0f);
        }

        return glm::vec3{ a[0], a[1], a[2] };
    }

    auto scale = readFloat(node);
    return glm::vec3{ scale };
}

glm::vec3 SceneFile::readRGB(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readFloatVector(node, 3);

        if (a.size() == 0) {
            a.push_back(0.f);
            a.push_back(0.f);
            a.push_back(0.f);
        }
        else if (a.size() == 1) {
            a.push_back(a[0]);
            a.push_back(a[0]);
        }
        else if (a.size() == 2) {
            a.push_back(a[0]);
        }

        return glm::vec3{ a[0], a[1], a[2] };
    }

    auto r = readFloat(node);
    return glm::vec3{ r, r, r };
}

glm::vec4 SceneFile::readRGBA(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        auto a = readFloatVector(node, 4);

        if (a.size() == 0) {
            a.push_back(0.f);
            a.push_back(0.f);
            a.push_back(0.f);
        }
        else if (a.size() == 1) {
            a.push_back(a[0]);
            a.push_back(a[0]);
        }
        else if (a.size() == 2) {
            a.push_back(a[0]);
        }

        // NOTE KI check if alpha is missing
        if (a.size() < 4) {
            a.push_back(DEF_ALPHA);
        }

        return glm::vec4{ a[0], a[1], a[2], a[3] };
    }

    auto r = readFloat(node);
    return glm::vec4{ r, r, r, DEF_ALPHA };
}

glm::vec2 SceneFile::readRefractionRatio(const YAML::Node& node) const
{
    auto a = readFloatVector(node, 2);

    // NOTE KI check if just single number
    if (a.size() < 1) {
        a.push_back(1.0);
    }
    return glm::vec2{ a[0], a[1] };
}

uuids::uuid SceneFile::resolveUUID(
    const BaseUUID& parts,
    const int cloneIndex,
    const glm::uvec3& tile)
{
    if (parts.empty()) {
        return {};
    }

    std::string key = parts[0];
    key = util::toUpper(key);

    if (key.empty()) return {};

    if (key == AUTO_UUID) {
        uuids::uuid uuid;
        if (parts.size() > 1) {
            std::string name = expandMacros(parts[1], cloneIndex, tile);

            {
                std::lock_guard<std::mutex> lock(uuid_lock);

                const auto& it = m_autoIds.find(name);
                if (it == m_autoIds.end()) {
                    uuid = uuids::uuid_system_generator{}();
                    m_autoIds[name] = uuid;
                }
                else {
                    uuid = it->second;
                }
            }
        }
        if (uuid.is_nil()) {
            uuid = uuids::uuid_system_generator{}();
        }
        return uuid;
    }
    else if (key == ROOT_UUID) {
        return m_assets.rootUUID;
    }
    else if (key == VOLUME_UUID) {
        return m_assets.volumeUUID;
    }
    else if (key == CUBE_MAP_UUID) {
        return m_assets.cubeMapUUID;
    }
    else {
        return KI_UUID(key);
    }
}

std::string SceneFile::expandMacros(
    const std::string& str,
    const int cloneIndex,
    const glm::uvec3& tile)
{
    std::string out{ str };

    {
        const auto pos = out.find("{x}");
        if (pos != std::string::npos) {
            out.replace(pos, 3, fmt::format("{}", tile.x));
        }
    }
    {
        const auto pos = out.find("{y}");
        if (pos != std::string::npos) {
            out.replace(pos, 3, fmt::format("{}", tile.y));
        }
    }
    {
        const auto pos = out.find("{z}");
        if (pos != std::string::npos) {
            out.replace(pos, 3, fmt::format("{}", tile.z));
        }
    }

    return out;
}

SceneFile::BaseUUID SceneFile::readUUID(const YAML::Node& node)
{
    BaseUUID parts;

    if (node.IsSequence()) {
        for (const auto& e : node) {
            parts.push_back(e.as<std::string>());
        }
    }
    else {
        parts.push_back(node.as<std::string>());
    }

    return parts;
}

const std::string SceneFile::resolveTexturePath(std::string_view path) const
{
    return std::string{ path };
}

std::string SceneFile::readFile(std::string_view filename) const
{
    std::stringstream buffer;

    std::string filePath = util::joinPath(
        m_dirname,
        filename);

    if (!util::fileExists(filePath)) {
        throw std::runtime_error{ fmt::format("FILE_NOT_EXIST: {}", filePath) };
    }

    try {
        std::ifstream t(filePath);
        t.exceptions(std::ifstream::badbit);
        //t.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        buffer << t.rdbuf();
    }
    catch (std::ifstream::failure e) {
        std::string what{ e.what() };
        const auto msg = fmt::format(
            "SCENE::FILE_NOT_SUCCESFULLY_READ: {}\n{}",
            filePath, what);

        throw std::runtime_error{ msg };
    }
    return buffer.str();
}

void SceneFile::reportUnknown(
    std::string_view scope,
    std::string_view k,
    const YAML::Node& v) const
{
    std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
    KI_WARN_OUT(fmt::format("{} {}: {}={}", prefix, scope, k, renderNode(v)));
}

std::string SceneFile::renderNode(
    const YAML::Node& v) const
{
    std::stringstream ss;
    ss << v;
    return ss.str();
}
