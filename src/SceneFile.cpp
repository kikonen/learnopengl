#include "SceneFile.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>
#include <string>
#include <filesystem>

#include <fmt/format.h>

#include "util/Util.h"

#include "asset/Material.h"
#include "asset/ModelMesh.h"
#include "asset/QuadMesh.h"
#include "asset/SpriteMesh.h"
#include "asset/Program.h"

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"

#include "model/Node.h"

#include "generator/GridGenerator.h"
#include "generator/TerrainGenerator.h"

#include "controller/AsteroidBeltController.h"
#include "controller/CameraController.h"
#include "controller/MovingLightController.h"
#include "controller/NodePathController.h"
#include "controller/VolumeController.h"

#include "registry/Registry.h"
#include "registry/MeshType.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/ModelRegistry.h"
#include "registry/MaterialRegistry.h"

#include "renderer/SkyboxRenderer.h"

#include "scene/LegacyTerrainGenerator.h"

#include <scene/AsyncLoader.h>


namespace {
    const double DEF_ALPHA = 1.0;

    const std::string AUTO_UUID{ "AUTO" };

    const std::string QUAD_MESH_NAME{ "quad" };
    const std::string SKYBOX_MESH_NAME{ "quad_skybox" };
}


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

    std::ifstream fin(m_filename);
    YAML::Node doc = YAML::Load(fin);

    loadSkybox(doc, m_skybox);
    loadMaterials(doc, m_materials);

    loadRoot(doc, m_root);
    loadEntities(doc, m_entities);

    attach(m_skybox, m_root, m_entities, m_materials);
}

void SceneFile::attach(
    SkyboxData& skybox,
    const EntityData& root,
    const std::vector<EntityData>& entities,
    std::vector<Material>& materials)
{
    attachSkybox(root, skybox, materials);

    attachEntity(root, root, materials);
    attachVolume(root);
    attachCubeMapCenter(root);

    for (const auto& entity : entities) {
        attachEntity(root, entity, materials);
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
        SKYBOX_MESH_NAME);
    auto* mesh = future.get();
    type->setMesh(mesh);
    type->m_entityType = EntityType::skybox;

    auto& flags = type->m_flags;

    flags.wireframe = false;
    flags.renderBack = true;
    flags.noShadow = true;
    flags.noFrustum = true;
    //flags.noReflect = true;
    //flags.noRefract = true;
    flags.noDisplay = false;
    flags.noSelect = true;

    type->m_drawOptions.depthFunc = GL_LEQUAL;

    type->m_program = m_registry->m_programRegistry->getProgram(data.programName);

    auto node = new Node(type);
    node->m_parentId = root.base.id;

    auto skybox = std::make_unique<SkyboxRenderer>(data.programName, data.materialName);
    m_registry->m_nodeRegistry->m_skybox = std::move(skybox);
    m_registry->m_nodeRegistry->addNode(node);
}

void SceneFile::attachVolume(
    const EntityData& root)
{
    if (!m_assets.showVolume) return;

    auto type = m_registry->m_typeRegistry->getType("<volume>");

    auto future = m_registry->m_modelRegistry->getMesh("ball_volume");
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

    type->m_program = m_registry->m_programRegistry->getProgram(TEX_VOLUME);

    auto node = new Node(type);
    node->m_id = m_assets.volumeUUID;
    node->m_parentId = root.base.id;

    // NOTE KI m_radius = 1.73205078
    mesh->prepareVolume();

    node->setAABB(mesh->getAABB());

    node->m_controller = std::make_unique<VolumeController>();

    m_registry->m_nodeRegistry->addNode(node);
}

void SceneFile::attachCubeMapCenter(
    const EntityData& root)
{
    if (!m_assets.showCubeMapCenter) return;

    auto type = m_registry->m_typeRegistry->getType("<cube_map>");

    auto future = m_registry->m_modelRegistry->getMesh("ball_volume");
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

    type->m_program = m_registry->m_programRegistry->getProgram(TEX_VOLUME);

    auto node = new Node(type);
    node->m_id = m_assets.cubeMapUUID;
    node->m_parentId = root.base.id;

    //node->setScale(m_asyncLoader->assets.cubeMapFarPlane);
    node->setScale(4.f);

    // NOTE KI m_radius = 1.73205078
    mesh->prepareVolume();

    node->setAABB(mesh->getAABB());

    m_registry->m_nodeRegistry->addNode(node);
}

void SceneFile::attachEntity(
    const EntityData& root,
    const EntityData& data,
    std::vector<Material>& materials)
{
    if (!data.base.enabled) {
        return;
    }

    m_asyncLoader->addLoader(m_alive, [this, &root, &data, &materials]() {
        if (data.clones.empty()) {
            MeshType* type{ nullptr };
            attachEntityClone(type, root, data, data.base, false, 0, materials);
        }
        else {
            MeshType* type{ nullptr };

            int cloneIndex = 0;
            for (auto& cloneData : data.clones) {
                if (!*m_alive) return;
                type = attachEntityClone(type, root, data, cloneData, true, cloneIndex, materials);
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
    std::vector<Material>& materials)
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
                const glm::vec3 posAdjustment{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                type = SceneFile::attachEntityCloneRepeat(
                    type,
                    root,
                    entity,
                    data,
                    cloned,
                    cloneIndex,
                    tile,
                    posAdjustment,
                    materials);

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
    const glm::vec3& posAdjustment,
    std::vector<Material>& materials)
{
    if (!*m_alive) return type;

    if (!data.enabled) {
        return type;
    }

    auto& nodeRegistry = *m_registry->m_nodeRegistry;

    if (!type) {
        type = createType(
            entity.isRoot,
            data,
            tile,
            materials);
        if (!type) return type;
    }

    if (!*m_alive) return type;

    auto node = createNode(
        type, root, data,
        cloned, cloneIndex, tile,
        data.clonePosition, posAdjustment,
        entity.isRoot);

    if (data.selected) {
        node->setSelectionMaterialIndex(nodeRegistry.m_selectionMaterial.m_registeredIndex);
    }

    nodeRegistry.addNode(node);

    return type;
}

MeshType* SceneFile::createType(
    bool isRoot,
    const EntityCloneData& data,
    const glm::uvec3& tile,
    std::vector<Material>& materials)
{
    auto type = m_registry->m_typeRegistry->getType(data.name);
    assignFlags(data, type);

    type->m_priority = data.priority;
    type->m_script = data.script;

    if (isRoot) {
        type->m_flags.root = true;
        type->m_flags.invisible = true;
        type->m_entityType = EntityType::origo;
    }

    if (data.instanced) {
        type->m_flags.instanced = true;
    }

    resolveMaterial(type, data, materials);
    resolveMesh(type, data, tile);

    if (data.type != EntityType::origo) {
        // NOTE KI container does not have mesh itself, but it can setup
        // material & program for contained nodes
        if (data.type != EntityType::container) {
            if (!type->getMesh()) {
                KI_WARN(fmt::format(
                    "SCENE_FILEIGNORE: NO_MESH id={} ({})",
                    KI_UUID_STR(data.id), data.name));
                return nullptr;
            }
        }

        bool normalTex = true;

        type->modifyMaterials([this, &normalTex, &data](Material& m) {
            if (data.materialModifiers_enabled) {
                modifyMaterial(m, data.materialModifierFields, data.materialModifiers);
            }
            m.loadTextures(m_assets);

            normalTex |= m.hasNormalTex();
        });

        if (!data.programName.empty()) {
            std::map<std::string, std::string> definitions;
            for (const auto& [k, v] : data.programDefinitions) {
                definitions[k] = v;
            }
            if (type->m_flags.alpha) {
                definitions[DEF_USE_ALPHA] = "1";
            }
            if (type->m_flags.blend) {
                definitions[DEF_USE_BLEND] = "1";
            }
            if (normalTex) {
                definitions[DEF_USE_NORMAL_TEX] = "1";
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

void SceneFile::resolveMesh(
    MeshType* type,
    const EntityCloneData& data,
    const glm::uvec3& tile)
{
    // NOTE KI materials MUST be resolved before loading mesh
    if (data.type == EntityType::model) {
        auto future = m_registry->m_modelRegistry->getMesh(
            data.meshName,
            data.meshPath);
        auto* mesh = future.get();
        type->setMesh(mesh);
        type->m_entityType = EntityType::model;

        KI_INFO(fmt::format(
            "SCENE_FILE ATTACH: id={}, type={}",
            data.id_str, type->str()));
    }
    else if (data.type == EntityType::quad) {
        auto mesh = std::make_unique<QuadMesh>();
        mesh->prepareVolume();
        type->setMesh(std::move(mesh), true);
        type->m_entityType = EntityType::quad;
    }
    else if (data.type == EntityType::billboard) {
        if (true) {
            auto future = m_registry->m_modelRegistry->getMesh(
                QUAD_MESH_NAME);
            auto* mesh = future.get();
            type->setMesh(mesh);
        }
        else {
            auto mesh = std::make_unique<QuadMesh>();
            mesh->prepareVolume();
            type->setMesh(std::move(mesh), true);
        }
        type->m_entityType = EntityType::billboard;
    }
    else if (data.type == EntityType::sprite) {
        // NOTE KI sprite *shall* differ from quad later on
        auto mesh = std::make_unique<SpriteMesh>();
        mesh->prepareVolume();
        type->setMesh(std::move(mesh), true);
        type->m_entityType = EntityType::sprite;
    }
    else if (data.type == EntityType::terrain) {
        Material material = type->m_materialVBO.m_defaultMaterial;
        LegacyTerrainGenerator generator(
            m_assets,
            data.tiling.tiles.z,
            data.tiling.tiles.x,
            data.tiling.heightScale);
        auto mesh = generator.generateTerrain(tile.z, tile.x, &material);
        type->setMesh(std::move(mesh), true);
        type->m_entityType = EntityType::terrain;
    }
    else if (data.type == EntityType::container) {
        // NOTE KI generator takes care of actual work
        type->m_entityType = EntityType::container;
        type->m_flags.invisible = true;
    }
    else if (data.type == EntityType::origo) {
        // NOTE KI nothing to do
        type->m_entityType = EntityType::origo;
        type->m_flags.invisible = true;
    }
    else {
        // NOTE KI unknown; don't render, just keep it in hierarchy
        type->m_entityType = EntityType::origo;
        type->m_flags.invisible = true;
    }
}

Node* SceneFile::createNode(
    MeshType* type,
    const EntityData& root,
    const EntityCloneData& data,
    bool cloned,
    int cloneIndex,
    const glm::uvec3& tile,
    const glm::vec3& clonePosition,
    const glm::vec3& posAdjustment,
    bool isRoot)
{
    Node* node = new Node(type);

    node->setCloneIndex(cloneIndex);
    node->setTile(tile);

    glm::vec3 pos = data.position + clonePosition + posAdjustment;

    // NOTE KI no id for clones; would duplicate base id => conflicts
    // => except if clone defines own ID
    if (root.base.id != data.id || isRoot)
        node->m_id = data.id;

    if (!isRoot) {
        if (data.parentId.is_nil()) {
            node->m_parentId = root.base.id;
        }
        else {
            node->m_parentId = data.parentId;
        }
    }

    node->setPosition(pos);
    node->setRotation(data.rotation);
    node->setScale(data.scale);

    node->setPlaneNormal(data.planeNormal);

    auto mesh = type->getMesh();
    if (mesh) {
        node->setAABB(mesh->getAABB());
    }

    if (data.camera.enabled) {
        node->m_camera = createCamera(data, data.camera);
    }

    if (data.light.enabled) {
        node->m_light = createLight(data, data.light);
    }

    if (data.controller.enabled) {
        node->m_controller = createController(data, data.controller, node);
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
            if (flags.blend) flags.alpha = true;
        }
    }
    {
        const auto& e = data.renderFlags.find("render_back");
        if (e != data.renderFlags.end()) {
            flags.renderBack = e->second;
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

    if (f.fogRatio) m.fogRatio = mod.fogRatio;

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
    if (f.ni) m.ni = mod.ni;
    if (f.d) m.d = mod.d;
    if (f.illum) m.illum = mod.illum;

    if (f.map_dudv) m.map_dudv = mod. map_dudv;
    if (f.map_height) m.map_height = mod.map_height;
}

std::unique_ptr<Camera> SceneFile::createCamera(
    const EntityCloneData& entity,
    const CameraData& data)
{
    if (!data.enabled) return std::unique_ptr<Camera>();

    // NOTE only node cameras in scenefile for now
    auto camera = std::make_unique<Camera>(data.pos, data.front, data.up, true);
    camera->setRotation(data.rotation);
    camera->setZoom(data.zoom);

    camera->setEnabled(data.enabled);
    camera->setDefault(data.isDefault);

    return camera;
}

std::unique_ptr<Light> SceneFile::createLight(
    const EntityCloneData& entity,
    const LightData& data)
{
    if (!data.enabled) return std::unique_ptr<Light>();

    auto light = std::make_unique<Light>();

    light->setPosition(data.pos);
    light->setTargetId(data.targetId);

    light->linear = data.linear;
    light->quadratic = data.quadratic;

    light->cutoffAngle = data.cutoffAngle;
    light->outerCutoffAngle = data.outerCutoffAngle;

    light->ambient = data.ambient;
    light->diffuse = data.diffuse;
    light->specular = data.specular;

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

std::unique_ptr<NodeController> SceneFile::createController(
    const EntityCloneData& entity,
    const ControllerData& data,
    Node* node)
{
    if (!data.enabled) return nullptr;

    const auto& center = node->getPosition();

    switch (data.type) {
        case ControllerType::camera: {
            auto controller = std::make_unique<CameraController>();
            return controller;
        }
        case ControllerType::path: {
            auto controller = std::make_unique<NodePathController>(center, data.mode);
            return controller;
        }
        case ControllerType::asteroid_belt: {
            auto controller = std::make_unique<AsteroidBeltController>(data.count);
            return controller;
        }
        case ControllerType::moving_light: {
            auto controller = std::make_unique<MovingLightController>(center, data.radius, data.speed);
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

        generator->m_worldTileSize = tiling.tile_size;
        generator->m_worldTilesZ = tiling.tiles.z;
        generator->m_worldTilesX = tiling.tiles.x;
        generator->m_heightScale= tiling.heightScale;
        generator->m_material = materialVBO.m_defaultMaterial;

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
        }
        else if (k == "material") {
            data.materialName = v.as<std::string>();
        }
        else if (k == "priority") {
            data.priority = v.as<int>();
        }
        else {
            reportUnknown("skybox_entry", k, v);
        }
    }
}

void SceneFile::loadRoot(
    const YAML::Node& doc,
    EntityData& root)
{
    auto& node = doc["root"];
    loadEntity(node, root);

    root.base.id_str = KI_UUID_STR(m_assets.rootUUID);
    root.base.id = m_assets.rootUUID;
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

        //std::cout << k << " = " << v << "\n";

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
            data.priority = v.as<int>();
        }
        else if (k == "name") {
            data.name = v.as<std::string>();
        }
        else if (k == "desc") {
            data.desc = v.as<std::string>();
        }
        else if (k == "id") {
            data.id_str = v.as<std::string>();
            if (!data.id_str.empty()) {
                if (util::toUpper(data.id_str) == AUTO_UUID) {
                    data.id_auto = true;
                    data.id = uuids::uuid_system_generator{}();
                }
                else {
                    data.id = KI_UUID(data.id_str);
                }
            }
        }
        else if (k == "parent_id") {
            data.parentId_str = v.as<std::string>();
            data.parentId = KI_UUID(data.parentId_str);
        }
        else if (k == "model") {
            if (v.Type() == YAML::NodeType::Sequence) {
                data.meshName = v[0].as<std::string>();
                data.meshPath = v[1].as<std::string>();
            }
            else {
                data.meshName = v.as<std::string>();
            }
        }
        else if (k == "program" || k == "shader") {
            data.programName = v.as<std::string>();
            if (data.programName == "texture") {
                data.programName = TEX_TEXTURE;
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
                const auto flagValue = flagNode.second.as<bool>();
                data.renderFlags[util::toLower(flagName)] = flagValue;
            }
        }
        else if (k == "plane_normal") {
            data.planeNormal = readVec3(v);
        }
        else if (k == "material") {
            data.materialName = v.as<std::string>();
        }
        else if (k == "material_modifier") {
            loadMaterialModifiers(v, data);
        }
        else if (k == "force_material") {
            data.forceMaterial = v.as<bool>();
        }
        else if (k == "batch_size") {
            data.batchSize = v.as<int>();
        }
        else if (k == "load_textures") {
            data.loadTextures = v.as<bool>();
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
            data.instanced = v.as<bool>();
        }
        else if (k == "selected") {
            data.selected = v.as<bool>();
        }
        else if (k == "enabled") {
            data.enabled = v.as<bool>();
        }
        else if (k == "clone_position") {
            data.clonePosition = readVec3(v);
        }
        else if (k == "clone_mesh") {
            data.cloneMesh = v.as<bool>();
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
            data.xCount = v.as<int>();
        }
        else if (k == "y_count") {
            data.yCount = v.as<int>();
        }
        else if (k == "z_count") {
            data.zCount = v.as<int>();
        }
        else if (k == "x_step") {
            data.xStep = v.as<double>();
        }
        else if (k == "y_step") {
            data.yStep = v.as<double>();
        }
        else if (k == "z_step") {
            data.zStep = v.as<double>();
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
            data.tile_size = v.as<int>();
        }
        else if (k == "height_scale") {
            data.heightScale = v.as<float>();
        }
        else {
            reportUnknown("tiling_entry", k, v);
        }
    }
}

void SceneFile::loadCamera(const YAML::Node& node, CameraData& data)
{
    data.zoom = m_assets.cameraFov;

    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = v.as<bool>();
        }
        else if (k == "default") {
            data.isDefault = v.as<bool>();
        }
        else if (k == "zoom") {
            data.zoom = v.as<float>();
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
        else {
            reportUnknown("controller_entry", k, v);
        }
    }
}

void SceneFile::loadLight(const YAML::Node& node, LightData& data)
{
    // Default to center
    data.targetId_str = KI_UUID_STR(m_assets.rootUUID);
    data.targetId = m_assets.rootUUID;

    // pos relative to owning node
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = v.as<bool>();
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
            data.targetId_str = v.as<std::string>();
            data.targetId = KI_UUID(data.targetId_str);
        }
        else if (k == "linear") {
            data.linear = v.as<float>();
        }
        else if (k == "quadratic") {
            data.quadratic = v.as<float>();
        }
        else if (k == "cutoff_angle") {
            data.cutoffAngle = v.as<float>();
        }
        else if (k == "outer_cutoff_angle") {
            data.outerCutoffAngle = v.as<float>();
        }
        else if (k == "ambient") {
            data.ambient = readRGBA(v);
        }
        else if (k == "diffuse") {
            data.diffuse = readRGBA(v);
        }
        else if (k == "specular") {
            data.specular = readRGBA(v);
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
            data.enabled = v.as<bool>();
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "none") {
                data.type = ControllerType::none;
            }
            else if (type == "camera") {
                data.type = ControllerType::camera;
            }
            else if (type == "path") {
                data.type = ControllerType::path;
            }
            else if (type == "asteroid_belt") {
                data.type = ControllerType::asteroid_belt;
            }
            else if (type == "moving_light") {
                data.type = ControllerType::moving_light;
            }
            else {
                reportUnknown("controller_type", k, v);
            }
        }
        //else if (k == "center") {
        //    data.center = readVec3(v);
        //}
        else if (k == "repeat") {
            loadRepeat(v, data.repeat);
        }
        else if (k == "speed") {
            data.speed = v.as<float>();
        }
        else if (k == "radius") {
            data.radius = v.as<float>();
        }
        else if (k == "mode") {
            data.mode = v.as<int>();
        }
        else if (k == "count") {
            data.count = v.as<int>();
        }
        else if (k == "pos") {
            //data.pos = readVec3(v);
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
            data.enabled = v.as<bool>();
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "none") {
                data.type = GeneratorType::none;
            }
            else if (type == "terrain") {
                data.type = GeneratorType::terrain;
            }
            else {
                reportUnknown("generator_type", k, v);
            }
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
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

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
            material.ns = v.as<float>();
            fields.ns = true;
        }
        else if (k == "ka") {
            material.ka = readRGBA(v);
            fields.ka = true;
        }
        else if (k == "kd") {
            material.kd = readRGBA(v);
            fields.kd = true;
        }
        else if (k == "ks") {
            material.ks = readRGBA(v);
            fields.ks = true;
        }
        else if (k == "ke") {
            material.ke = readRGBA(v);
            fields.ke = true;
        }
        else if (k == "ni") {
            material.ni = v.as<float>();
            fields.ni = true;
        }
        else if (k == "d") {
            material.d = v.as<float>();
            fields.d = true;
        }
        else if (k == "illum") {
            material.d = v.as<float>();
            fields.illum = true;
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
        else if (k == "map_bump") {
            std::string line = v.as<std::string>();
            material.map_bump = resolveTexturePath(line);
            fields.map_bump = true;
        }
        else if (k == "bump") {
            std::string line = v.as<std::string>();
            material.map_bump = resolveTexturePath(line);
            fields.map_bump = true;
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
        else if (k == "pattern") {
            material.pattern = v.as<int>();
            fields.pattern = true;
        }
        else if (k == "reflection") {
            material.reflection = v.as<float>();
            fields.reflection = true;
        }
        else if (k == "refraction") {
            material.refraction = v.as<float>();
            fields.refraction = true;
        }
        else if (k == "refraction_ratio") {
            material.refractionRatio = readRefractionRatio(v);
            fields.refractionRatio = true;
        }
        else if (k == "fog_ratio") {
            material.fogRatio = v.as<float>();
            fields.fogRatio = true;
        }
        else if (k == "tiling") {
            material.tilingX = v.as<float>();
            material.tilingY = v.as<float>();
            fields.tilingX = true;
            fields.tilingY = true;
        }
        else if (k == "tilingX") {
            material.tilingX = v.as<float>();
            fields.tilingX = true;
        }
        else if (k == "tilingY") {
            material.tilingY = v.as<float>();
            fields.tilingY = true;
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
    reportUnknown("wrap_mode", k, v);
}

glm::vec2 SceneFile::readVec2(const YAML::Node& node) const
{
    std::vector<float> a;
    a.reserve(2);

    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    return glm::vec2{ a[0], a[1] };
}

glm::vec3 SceneFile::readVec3(const YAML::Node& node) const
{
    std::vector<double> a;
    a.reserve(3);

    for (const auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 SceneFile::readVec4(const YAML::Node& node) const
{
    std::vector<float> a;
    a.reserve(4);

    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::uvec3 SceneFile::readUVec3(const YAML::Node& node) const
{
    std::vector<int> a;
    a.reserve(3);

    for (const auto& e : node) {
        a.push_back(e.as<int>());
    }
    return glm::uvec3{ a[0], a[1], a[2] };
}

glm::vec3 SceneFile::readScale3(const YAML::Node& node) const
{
    if (node.IsSequence()) {
        std::vector<float> a;
        a.reserve(3);

        for (const auto& e : node) {
            a.push_back(e.as<float>());
        }
        while (a.size() < 3) {
            a.push_back(1.0);
        }
        return glm::vec3{ a[0], a[1], a[2] };
    }

    auto scale = node.as<float>();
    return glm::vec3{ scale };
}

glm::vec4 SceneFile::readRGBA(const YAML::Node& node) const
{
    std::vector<float> a;
    a.reserve(4);

    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    // NOTE KI check if alpha is missing
    if (a.size() < 4) {
        a.push_back(DEF_ALPHA);
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec2 SceneFile::readRefractionRatio(const YAML::Node& node) const
{
    std::vector<float> a;
    a.reserve(2);

    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    // NOTE KI check if just single number
    if (a.size() < 1) {
        a.push_back(1.0);
    }
    return glm::vec2{ a[0], a[1] };
}

const std::string SceneFile::resolveTexturePath(const std::string& path) const
{
    return path;
}

std::string SceneFile::readFile(const std::string& filename) const
{
    std::stringstream buffer;

    std::filesystem::path filePath;
    filePath /= m_dirname;
    filePath /= filename;
    try {
        std::ifstream t(filePath);
        t.exceptions(std::ifstream::failbit | std::ifstream::badbit);

        buffer << t.rdbuf();
    }
    catch (std::ifstream::failure e) {
        std::string what{ e.what() };
        const auto msg = fmt::format(
            "SCENE::FILE_NOT_SUCCESFULLY_READ: {}\n{}",
            filePath.string(), what);

        throw std::runtime_error{ msg };
    }
    return buffer.str();
}

void SceneFile::reportUnknown(
    const std::string& scope,
    const std::string& k,
    const YAML::Node& v) const
{
    std::string prefix = k.starts_with("xx") ? "DISABLED" : "UNKNOWN";
    std::stringstream ss;
    ss<< v;
    KI_WARN_OUT(fmt::format("{} {}: {}={}", prefix, scope, k, ss.str()));
}
