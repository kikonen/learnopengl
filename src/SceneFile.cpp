#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <fmt/format.h>

#include "asset/MeshLoader.h"
#include "asset/QuadMesh.h"

#include "model/InstancedNode.h"

#include "controller/AsteroidBeltController.h"
#include "controller/CameraController.h"
#include "controller/MovingLightController.h"
#include "controller/NodePathController.h"

#include "SceneFile.h"

namespace {
    const double DEF_ALPHA = 1.0;
}

SceneFile::SceneFile(
    AsyncLoader* asyncLoader,
    const Assets& assets,
    const std::string& filename)
    : m_filename(filename),
    m_assets(assets),
    m_asyncLoader(asyncLoader)
{
}

SceneFile::~SceneFile()
{
}

void SceneFile::load(std::shared_ptr<Scene> scene)
{
    std::ifstream fin(m_filename);
    YAML::Node doc = YAML::Load(fin);

    loadSkybox(doc, m_skybox);
    loadMaterials(doc, m_materials);

    loadRoot(doc, m_root);
    loadEntities(doc, m_entities);

    attach(scene, m_skybox, m_root, m_entities, m_materials);
}

void SceneFile::attach(
    std::shared_ptr<Scene> scene,
    SkyboxData& skybox,
    const EntityData& root,
    const std::vector<EntityData>& entities,
    std::vector<Material>& materials)
{
    attachSkybox(scene, skybox, materials);

    attachEntity(scene, root, root, materials);

    for (const auto& entity : entities) {
        attachEntity(scene, root, entity, materials);
    }
}

void SceneFile::attachSkybox(
    std::shared_ptr<Scene> scene,
    SkyboxData& data,
    std::vector<Material>& materials)
{
    if (!data.valid()) return;

    auto skybox = std::make_unique<SkyboxRenderer>(data.shaderName, data.materialName);
    skybox->prepare(m_assets, m_asyncLoader->shaders);
    scene->skyboxRenderer.reset(skybox.release());
}

void SceneFile::attachEntity(
    std::shared_ptr<Scene> scene,
    const EntityData& root,
    const EntityData& data,
    std::vector<Material>& materials)
{
    if (!data.base.enabled) {
        return;
    }

    m_asyncLoader->addLoader([this, scene, &root, &data, &materials]() {
        if (data.clones.empty()) {
            attachEntityClone(scene, root, data, data.base, false, materials);
        }
        else {
            for (auto& cloneData : data.clones) {
                attachEntityClone(scene, root, data, cloneData, true, materials);
            }
        }
    });
}

void SceneFile::attachEntityClone(
    std::shared_ptr<Scene> scene,
    const EntityData& root,
    const EntityData& entity,
    const EntityCloneData& data,
    bool cloned,
    std::vector<Material>& materials)
{
    if (!data.enabled) {
        return;
    }

    const Assets& assets = m_assets;

    // NOTE KI if repeated then create transparent owner node for children
    const auto& repeat = data.repeat;
    const bool grouped = repeat.xCount > 1 || repeat.yCount > 1 || repeat.zCount > 1;

    auto type = std::make_shared<NodeType>();
    assignFlags(data, *type);
    {
        std::vector<std::string> definitions;
        for (auto& v : data.shaderDefinitions) {
            definitions.push_back(v);
        }
        if (type->flags.alpha) {
            definitions.push_back(DEF_USE_ALPHA);
        }
        if (type->flags.blend) {
            definitions.push_back(DEF_USE_BLEND);
        }

        type->nodeShader = m_asyncLoader->getShader(data.shaderName, definitions);
    }
    type->initScript = data.initScript;
    type->runScript = data.runScript;
    type->batch.batchSize = data.batchSize;
    type->flags.root = entity.isRoot;

    // NOTE KI need to create copy *IF* modifiers
    // TODO KI should make copy *ALWAYS* for safety
    Material* material = nullptr;
    if (!data.materialName.empty()) {
        material = Material::find(data.materialName, materials);
    }

    if (data.type == EntityType::model) {
        MeshLoader meshLoader(assets, data.name, data.meshName, data.meshPath);

        if (material) {
            meshLoader.defaultMaterial = *material;
        }
        meshLoader.forceDefaultMaterial = data.forceMaterial;
        meshLoader.loadTextures = data.loadTextures;

        auto mesh = meshLoader.load();
        KI_INFO_SB("SCENE_FILE ATTACH: id=" << data.id << " type = " << type->typeID << ", mesh = " << mesh->str());
        type->mesh.reset(mesh.release());
    }
    else if (data.type == EntityType::quad) {
        auto mesh = std::make_unique<QuadMesh>(data.name);
        if (material) {
            mesh->m_material = *material;
            if (data.loadTextures) {
                mesh->m_material.loadTextures(assets);
            }
        }
        type->mesh.reset(mesh.release());
    }
    else if (data.type == EntityType::sprite) {
        // NOTE KI sprite *shall* differ from quad later on
        auto mesh = std::make_unique<QuadMesh>(data.name);
        if (material) {
            mesh->m_material = *material;
            if (data.loadTextures) {
                mesh->m_material.loadTextures(assets);
            }
        }
        type->mesh.reset(mesh.release());
    }
    else if (data.type == EntityType::origo) {
        // NOTE KI nothing to do
        type->flags.origo = true;
    }

    if (data.type != EntityType::origo) {
        if (!type->mesh) {
            KI_WARN_SB("SCENE_FILEIGNORE: NO_MESH id=" << data.id << " (" << data.name << ")");
            return;
        }

        type->modifyMaterials([this, &data, &assets](Material& m) {
            modifyMaterial(m, data.materialModifierFields, data.materialModifiers);

            // NOTE KI if textures were not loaded, then need to load them now
            if (!data.loadTextures) {
                m.loadTextures(assets);
            }
            });
    }

    Group* group = nullptr;
    if (grouped) {
        group = new Group();
        group->id = data.id;
        scene->registry.addGroup(group);
    }

    for (auto z = 0; z < repeat.zCount; z++) {
        for (auto y = 0; y < repeat.yCount; y++) {
            for (auto x = 0; x < repeat.xCount; x++) {
                const glm::vec3 posAdjustment{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };
                auto node = createNode(group, root, data, type, data.clonePosition, posAdjustment, entity.isRoot, cloned);
                scene->registry.addNode(type.get(), node);
            }
        }
    }
}

Node* SceneFile::createNode(
    const Group* group,
    const EntityData& root,
    const EntityCloneData& data,
    const std::shared_ptr<NodeType>& type,
    const glm::vec3& clonePosition,
    const glm::vec3& posAdjustment,
    bool isRoot,
    bool cloned)
{
    auto node = data.instanced
        ? new InstancedNode(type)
        : new Node(type);

    if (data.instanced) {
        type->flags.instanced = true;
        type->flags.batchMode = false;
    }

    glm::vec3 pos = data.position + clonePosition + posAdjustment;

    if (group) {
        node->groupId = group->id;
    } else {
        // NOTE KI no id for clones; would duplicate base id => conflicts
        if (!cloned)
            node->id = data.id;
    }

    if (!isRoot) {
        if (data.parentId.is_nil()) {
            node->parentId = root.base.id;
        }
        else {
            node->parentId = data.parentId;
        }
    }

    node->setPos(pos);
    node->setRotation(data.rotation);
    node->setScale(data.scale);

    node->selected = data.selected;

    if (data.camera.enabled) {
        node->camera = createCamera(data, data.camera);
    }

    if (data.light.enabled) {
        node->light = createLight(data, data.light);
        type->flags.light = true;
    }

    if (data.controller.enabled) {
        node->controller = createController(data, data.controller, node);
    }

    return node;
}

void SceneFile::assignFlags(
    const EntityCloneData& data,
    NodeType& type)
{
    NodeRenderFlags& flags = type.flags;

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
        const auto& e = data.renderFlags.find("mirror");
        if (e != data.renderFlags.end()) {
            flags.mirror = e->second;
            type.mirrorPlane = data.mirrorPlane;
        }
    }
    {
        const auto& e = data.renderFlags.find("water");
        if (e != data.renderFlags.end()) {
            flags.water = e->second;
        }
    }
    {
        const auto& e = data.renderFlags.find("light");
        if (e != data.renderFlags.end()) {
            flags.light = e->second;
        }
    }
    {
        const auto& e = data.renderFlags.find("batch_mode");
        if (e != data.renderFlags.end()) {
            flags.batchMode = e->second;
        }
    }
    {
        const auto& e = data.renderFlags.find("wireframe");
        if (e != data.renderFlags.end()) {
            flags.wireframe = e->second;
        }
    }
}

void SceneFile::modifyMaterial(
    Material& m,
    const MaterialField& f,
    const Material& mod)
{
    if (f.type) m.type = mod.type;

    if (f.textureSpec) m.textureSpec = mod.textureSpec;

    if (f.pattern) m.pattern = mod.pattern;
    if (f.reflection) m.reflection = mod.reflection;
    if (f.refraction) m.refraction = mod.refraction;
    if (f.refractionRatio) m.refractionRatio = mod.refractionRatio;

    if (f.fogRatio) m.fogRatio = mod.fogRatio;

    if (f.tiling) m.tiling = mod.tiling;

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
}

std::unique_ptr<Camera> SceneFile::createCamera(
    const EntityCloneData& entity,
    const CameraData& data)
{
    if (!data.enabled) return std::unique_ptr<Camera>();

    auto pos = entity.position + data.pos;// +assets.groundOffset;
    auto camera = std::make_unique<Camera>(pos, data.front, data.up);
    camera->setRotation(data.rotation);

    return camera;
}

std::unique_ptr<Light> SceneFile::createLight(
    const EntityCloneData& entity,
    const LightData& data)
{
    if (!data.enabled) return std::unique_ptr<Light>();

    auto light = std::make_unique<Light>();

    light->setPos(data.pos);
    light->setWorldTarget(data.worldTarget);// +assets.groundOffset);

    light->linear = data.linear;
    light->quadratic = data.quadratic;

    light->cutoffAngle = data.cutoffAngle;
    light->outerCutoffAngle = data.outerCutoffAngle;

    light->ambient = data.ambient;
    light->diffuse = data.diffuse;
    light->specular = data.specular;

    switch (data.type) {
    case LightType::directional:
        light->directional = true;
        break;
    case LightType::point:
        light->point = true;
        break;
    case LightType::spot:
        light->spot = true;
        break;
    }

    return light;
}

std::unique_ptr<NodeController> SceneFile::createController(
    const EntityCloneData& entity,
    const ControllerData& data,
    Node* node)
{
    std::unique_ptr<NodeController> controller;

    if (!data.enabled) return controller;

    const auto center = node->getPos();

    switch (data.type) {
        case ControllerType::camera: {
            controller = std::make_unique<CameraController>();
            break;
        }
        case ControllerType::path: {
            controller = std::make_unique<NodePathController>(center, data.mode);
            break;
        }
        case ControllerType::asteroid_belt: {
            controller = std::make_unique<AsteroidBeltController>(data.count);
            break;
        }
        case ControllerType::moving_light: {
            controller = std::make_unique<MovingLightController>(center, data.radius, data.speed);
            break;
        }
    }

    return controller;
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

        if (k == "shader") {
            data.shaderName = v.as<std::string>();
        }
        else if (k == "material") {
            data.materialName = v.as<std::string>();
        }
        else {
            std::cout << "UNKNOWN SKYBOX_ENTRY: " << k << "=" << v << "\n";
        }
    }
}

void SceneFile::loadRoot(
    const YAML::Node& doc,
    EntityData& root)
{
    auto& node = doc["root"];
    loadEntity(node, root);

    root.base.type == EntityType::origo;
    root.isRoot = true;
    root.base.enabled = true;
}

void SceneFile::loadEntities(
    const YAML::Node& doc,
    std::vector<EntityData>& entities)
{
    for (const auto& entry : doc["entities"]) {
        EntityData data;
        loadEntity(entry, data);
        entities.push_back(data);
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
    const YAML::Node* clonesNode = nullptr;

    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        //std::cout << k << " = " << v << "\n";

        if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "origo") {
                data.type = EntityType::origo;
            }
            else if (type == "model") {
                data.type = EntityType::model;
            }
            else if (type == "quad") {
                data.type = EntityType::quad;
            }
            else if (type == "sprite") {
                data.type = EntityType::sprite;
            }
            else {
                std::cout << "UNKNOWN ENTITY_TYPE: " << k << "=" << v << "\n";
            }
        }
        else if (k == "name") {
            data.name = v.as<std::string>();
        }
        else if (k == "desc") {
            data.desc = v.as<std::string>();
        }
        else if (k == "id") {
            data.id_str = v.as<std::string>();
            data.id = KI_UUID(data.id_str);
        }
        else if (k == "parent_id") {
            data.parentId_str = v.as<std::string>();
            data.parentId = uuids::uuid::from_string(data.parentId_str).value();
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
        else if (k == "shader") {
            data.shaderName = v.as<std::string>();
            if (data.shaderName == "texture") {
                data.shaderName = TEX_TEXTURE;
            }
        }
        else if (k == "shader_definitions") {
            if (v.Type() == YAML::NodeType::Sequence) {
                for (const auto& name : v) {
                    data.shaderDefinitions.push_back(name.as<std::string>());
                }
            }
        }
        else if (k == "render_flags") {
            if (v.Type() == YAML::NodeType::Sequence) {
                for (const auto& name : v) {
                    auto flag = name.as<std::string>();
                    data.renderFlags[flag] = true;
                }
            }
        }
        else if (k == "mirror_plane") {
            data.mirrorPlane = readVec4(v);
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
        else if (k == "camera") {
            loadCamera(v, data.camera);
        }
        else if (k == "light") {
            loadLight(v, data.light);
        }
        else if (k == "controller") {
            loadController(v, data.controller);
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
        else if (k == "clones") {
            if (recurse)
                clonesNode = &v;
        }
        else if (k == "init_script") {
            data.initScript = v.as<std::string>();
        }
        else if (k == "run_script") {
            data.runScript = v.as<std::string>();
        }
        else {
            std::cout << "UNKNOWN ENTITY_ENTRY: " << k << "=" << v << "\n";
        }
    }

    if (clonesNode) {
        for (const auto& node : *clonesNode) {
            // NOTE KI intialize with current data
            EntityCloneData clone = data;
            std::vector<EntityCloneData> dummy{};
            loadEntityClone(node, clone, dummy, false);
            clones.push_back(clone);
        }
    }
}

void SceneFile::loadMaterialModifiers(
    const YAML::Node& node,
    EntityCloneData& data)
{
    data.materialModifiers.name = "<modifier>";

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
            std::cout << "UNKNOWN REPEAT_ENTRY: " << k << "=" << v << "\n";
        }
    }
}

void SceneFile::loadCamera(const YAML::Node& node, CameraData& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "enabled") {
            data.enabled = v.as<bool>();
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
            std::cout << "UNKNOWN CONTROLLER_ENTRY: " << k << "=" << v << "\n";
        }
    }
}

void SceneFile::loadLight(const YAML::Node& node, LightData& data)
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
                std::cout << "UNKNOWN LIGHT_TYPE: " << k << "=" << v << "\n";
            }
        }
        else if (k == "pos") {
            data.pos = readVec3(v);
        }
        else if (k == "world_target") {
            data.worldTarget = readVec3(v);
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
            std::cout << "UNKNOWN CONTROLLER_ENTRY: " << k << "=" << v << "\n";
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
                std::cout << "UNKNOWN CONTROLLER_TYPE: " << k << "=" << v << "\n";
            }
        }
        //else if (k == "center") {
        //    data.center = readVec3(v);
        //}
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
            std::cout << "UNKNOWN CONTROLLER_ENTRY: " << k << "=" << v << "\n";
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
            material.name = v.as<std::string>();
        }
        else if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "model") {
                material.type = MaterialType::model;
                fields.type = true;
            }
            else if (type == "texture") {
                material.type = MaterialType::texture;
                fields.type = true;
            }
            else if (type == "sprite") {
                material.type = MaterialType::sprite;
                fields.type = true;
            }
            else {
                std::cout << "UNKNOWN MATERIAL_TYPE: " << k << "=" << v << "\n";
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
            material.tiling = v.as<float>();
            fields.tiling = true;
        }
        else if (k == "texture_spec") {
            loadTextureSpec(v, material.textureSpec);
            fields.textureSpec = true;
        }
        else {
            std::cout << "UNKNOWN MATERIAL_ENTRY: " << k << "=" << v << "\n";
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

        if (k == "mode") {
            if (v.as<std::string>() == "GL_REPEAT") {
                textureSpec.mode = GL_REPEAT;
            } else {
                std::cout << "UNKNOWN MODE: " << k << "=" << v << "\n";
            }
        }
        else {
            std::cout << "UNKNOWN TEXTURE_SPEC: " << k << "=" << v << "\n";
        }
    }
}

glm::vec2 SceneFile::readVec2(const YAML::Node& node) {
    std::vector<float> a;
    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    return glm::vec2{ a[0], a[1] };
}

glm::vec3 SceneFile::readVec3(const YAML::Node& node) {
    std::vector<double> a;
    for (const auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 SceneFile::readVec4(const YAML::Node& node) {
    std::vector<float> a;
    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec3 SceneFile::readScale3(const YAML::Node& node) {
    std::vector<float> a;

    if (node.IsSequence()) {
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

glm::vec4 SceneFile::readRGBA(const YAML::Node& node) {
    std::vector<float> a;
    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    // NOTE KI check if alpha is missing
    if (a.size() < 4) {
        a.push_back(DEF_ALPHA);
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec2 SceneFile::readRefractionRatio(const YAML::Node& node) {
    std::vector<float> a;
    for (const auto& e : node) {
        a.push_back(e.as<float>());
    }
    // NOTE KI check if just single number
    if (a.size() < 1) {
        a.push_back(1.0);
    }
    return glm::vec2{ a[0], a[1] };
}

const std::string SceneFile::resolveTexturePath(const std::string& path)
{
    return path;
}
