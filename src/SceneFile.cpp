#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "asset/MeshLoader.h"
#include "asset/QuadMesh.h"

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
    : filename(filename),
    assets(assets),
    asyncLoader(asyncLoader)
{
}

SceneFile::~SceneFile()
{
}

void SceneFile::load(std::shared_ptr<Scene> scene)
{
    std::ifstream fin(filename);
    YAML::Node doc = YAML::Load(fin);

    loadSkybox(doc, skybox, materials);
    loadMaterials(doc, materials);
    loadEntities(doc, entities, materials);

    attach(scene, skybox, entities, materials);
}

void SceneFile::attach(
    std::shared_ptr<Scene> scene,
    const SkyboxData& skybox,
    const std::map<const uuids::uuid, EntityData>& entities,
    std::vector<Material>& materials)
{
    attachSkybox(scene, skybox, materials);

    for (const auto& entry : entities) {
        attachEntity(scene, entry.second, entities, materials);
    }
}

void SceneFile::attachSkybox(
    std::shared_ptr<Scene> scene,
    const SkyboxData& data,
    std::vector<Material>& materials)
{
    if (!skybox.valid()) return;

    auto skybox = std::make_unique<SkyboxRenderer>(data.shaderName, data.materialName);
    skybox->prepare(assets, asyncLoader->shaders);
    scene->skyboxRenderer.reset(skybox.release());
}

void SceneFile::attachEntity(
    std::shared_ptr<Scene> scene,
    const EntityData& data,
    const std::map<const uuids::uuid, EntityData>& entities,
    std::vector<Material>& materials)
{
    if (!data.enabled) {
        return;
    }

    EntityData parent;
    if (!data.parentId.is_nil()) {
        const auto& entry = entities.find(data.parentId);
        if (entry != entities.end()) {
            auto& e = entry->second;
            parent = e;
        }
    }

    //auto asyncLoader = this->asyncLoader;

    asyncLoader->addLoader([this, scene, &data, parent]() {
        const Assets& assets = asyncLoader->assets;

        auto type = std::make_shared<NodeType>(
            data.typeId,
            asyncLoader->getShader(data.shaderName, data.shaderDefinitions));

        type->batch.batchSize = data.batchSize;

        {
            NodeRenderFlags& flags = type->flags;
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
                    type->mirrorPlane = data.mirrorPlane;
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

        // NOTE KI need to create copy *IF* modifiers
        // TODO KI should make copy *ALWAYS* for safety
        Material* material = nullptr;
        if (!data.materialName.empty()) {
            material = Material::find(data.materialName, this->materials);
        }

        if (data.type == EntityType::model) {
            MeshLoader meshLoader(assets, data.modelName, data.modelPath);

            if (material) {
                meshLoader.defaultMaterial = *material;
                meshLoader.overrideMaterials = data.overrideMaterials;
            }
            meshLoader.loadTextures = data.loadTextures;

            auto mesh = meshLoader.load();
            KI_INFO_SB("SCENE_FILE ATTACH: id=" << data.id << " type = " << type->typeID << ", mesh = " << mesh->modelName);
            type->mesh.reset(mesh.release());
        }
        else if (data.type == EntityType::quad) {
            auto mesh = std::make_unique<QuadMesh>(data.name);
            if (material) {
                mesh->material = *material;
                mesh->material.loadTextures(assets);
            }
            type->mesh.reset(mesh.release());
        }
        else if (data.type == EntityType::sprite) {
        }

        if (!type->mesh) {
            KI_WARN_SB("SCENE_FILEIGNORE: NO_MESH id=" << data.id << " (" << data.name << ")");
            return;
        }

        type->modifyMaterials([this, &data, &assets](Material& m) {
            if (data.materialModifierFields.reflection) {
                m.reflection = data.materialModifiers->reflection;
            }
            if (data.materialModifierFields.refraction) {
                m.refraction = data.materialModifiers->refraction;
            }
            if (data.materialModifierFields.refractionRatio) {
                m.refractionRatio = data.materialModifiers->refractionRatio;
            }
            if (data.materialModifierFields.tiling) {
                m.tiling = data.materialModifiers->tiling;
            }
            if (data.materialModifierFields.textureSpec) {
                m.textureSpec = data.materialModifiers->textureSpec;
            }

            // NOTE KI if textures were not loaded, then need to load them now
            if (!data.loadTextures) {
                m.loadTextures(assets);
            }
         });

        const auto& repeat = data.repeat;
        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    for (const auto& p : data.positions) {
                        glm::vec3 pos = p;
                        // TODO KI let parent handling be problem of Scene Render logic
                        //  => i.e. that is what in reality need to do anyway
                        if (parent.valid) {
                            pos += parent.positions[0];
                        }
                        pos += glm::vec3{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                        auto node = new Node(type);
                        node->id = data.id;
                        node->parentId = data.parentId;

                        node->setPos(pos + assets.groundOffset);
                        node->setRotation(data.rotation);
                        node->setScale(data.scale);

                        node->selected = data.selected;

                        if (data.camera.enabled)    
                            node->camera = createCamera(data, data.camera);
                        if (data.light.enabled)
                            node->light = createLight(data, data.light);
                        if (data.controller.enabled)
                            node->controller = createController(data, data.controller);

                        scene->registry.addNode(node);
                    }
                }
            }
        }
     });
}

std::unique_ptr<Camera> SceneFile::createCamera(
    const EntityData& entity,
    const CameraData& data)
{
    if (!data.enabled) return std::unique_ptr<Camera>();

    auto pos = entity.positions[0] + data.pos + assets.groundOffset;
    auto camera = std::make_unique<Camera>(pos, data.front, data.up);
    camera->setRotation(data.rotation);

    return camera;
}

std::unique_ptr<Light> SceneFile::createLight(
    const EntityData& entity,
    const LightData& data)
{
    if (!data.enabled) return std::unique_ptr<Light>();

    switch (data.type) {
    case LightType::directional:
        //return std::unique_ptr<CameraController>();
        return std::unique_ptr<Light>();
    case LightType::point:
        return std::unique_ptr<Light>();
    case LightType::spot:
        return std::unique_ptr<Light>();
    }

    return std::unique_ptr<Light>();
}

std::unique_ptr<NodeController> SceneFile::createController(
    const EntityData& entity,
    const ControllerData& data)
{
    if (!data.enabled) return std::unique_ptr<NodeController>();

    switch (data.type) {
        case ControllerType::camera: {
            auto camera = std::make_unique<CameraController>();
            return std::move(camera);
        }
        case ControllerType::path: {
            auto path = std::make_unique<NodePathController>(1);
            return path;
        }
    }

//    return std::unique_ptr<NodeController>();
}

void SceneFile::loadSkybox(
    const YAML::Node& doc,
    SkyboxData& data,
    std::vector<Material>& materials)
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

void SceneFile::loadEntities(
    const YAML::Node& doc,
    std::map<const uuids::uuid, EntityData>& entities,
    std::vector<Material>& materials)
{
    for (const auto& entry : doc["entities"]) {
        EntityData data;
        loadEntity(entry, materials, data);
        // NOTE KI ignore elements without ID
        if (data.id.is_nil()) continue;
        data.valid = true;
        entities[data.id] = data;
    }
}

void SceneFile::loadEntity(
    const YAML::Node& node,
    std::vector<Material>& materials,
    EntityData& data)
{
    for (const auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        //std::cout << k << " = " << v << "\n";

        if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "model") {
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
            data.id = uuids::uuid::from_string(v.as<std::string>()).value();
        }
        else if (k == "type_id") {
            //data.typeId = v.as<int>();
            data.typeId = NodeType::nextID();
        }
        else if (k == "parent_id") {
            data.parentId = uuids::uuid::from_string(v.as<std::string>()).value();
        }
        else if (k == "model") {
            if (v.Type() == YAML::NodeType::Sequence) {
                data.modelName = v[0].as<std::string>();
                data.modelPath = v[1].as<std::string>();
            }
            else {
                data.modelName = v.as<std::string>();
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
        else if (k == "override_material") {
            data.overrideMaterials = v.as<bool>();
        }
        else if (k == "batch_size") {
            data.batchSize = v.as<int>();
        }
        else if (k == "load_textures") {
            data.loadTextures = v.as<bool>();
        }
        else if (k == "pos") {
            data.positions.push_back(readVec3(v));
        }
        else if (k == "positions") {
            data.positions.clear();
            for (const auto& p : v) {
                data.positions.push_back(readVec3(p));
            }
        }
        else if (k == "rotation") {
            data.rotation = readVec3(v);
        }
        else if (k == "scale") {
            data.scale = v.as<double>();
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
        else if (k == "selected") {
            data.selected = v.as<bool>();
        }
        else if (k == "enabled") {
            data.enabled = v.as<bool>();
        }
        else {
            std::cout << "UNKNOWN ENTITY_ENTRY: " << k << "=" << v << "\n";
        }
    }

    if (data.positions.empty()) {
        // NOTE KI *ENSURE* there is position
        data.positions.emplace_back(glm::vec3(0));
    }
}

void SceneFile::loadMaterialModifiers(
    const YAML::Node& node,
    EntityData& data)
{
    data.materialModifiers = std::make_shared<Material>();
    data.materialModifiers->name = "<modifier>";

    loadMaterial(node, data.materialModifierFields, *data.materialModifiers);
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
        else if (k == "target") {
            data.target = readVec3(v);
        }
        else if (k == "ambient") {
            data.ambient = readVec3(v);
        }
        else if (k == "diffuse") {
            data.diffuse = readVec3(v);
        }
        else if (k == "specular") {
            data.specular = readVec3(v);
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
            else {
                std::cout << "UNKNOWN CONTROLLER_TYPE: " << k << "=" << v << "\n";
            }
        }
        else if (k == "pos") {
    //        data.pos = readVec3(v);
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
            }
            else if (type == "texture") {
                material.type = MaterialType::texture;
            }
            else if (type == "sprite") {
                material.type = MaterialType::sprite;
            }
            else {
                std::cout << "UNKNOWN MATERIAL_TYPE: " << k << "=" << v << "\n";
            }
        }
        else if (k == "ns") {
            material.ns = v.as<float>();
        }
        else if (k == "ka") {
            material.ka = readRGBA(v);
        }
        else if (k == "kd") {
            material.kd = readRGBA(v);
        }
        else if (k == "ks") {
            material.ks = readRGBA(v);
        }
        else if (k == "ke") {
            material.ke = readRGBA(v);
        }
        else if (k == "ni") {
            material.ni = v.as<float>();
        }
        else if (k == "d") {
            material.d = v.as<float>();
        }
        else if (k == "illum") {
            material.d = v.as<float>();
        }
        else if (k == "map_kd") {
            std::string line = v.as<std::string>();
            material.map_kd = resolveTexturePath(line);
        }
        else if (k == "map_ke") {
            std::string line = v.as<std::string>();
            material.map_ke = resolveTexturePath(line);
        }
        else if (k == "map_ks") {
            std::string line = v.as<std::string>();
            material.map_ks = resolveTexturePath(line);
        }
        else if (k == "map_bump") {
            std::string line = v.as<std::string>();
            material.map_bump = resolveTexturePath(line);
        }
        else if (k == "bump") {
            std::string line = v.as<std::string>();
            material.map_bump = resolveTexturePath(line);
        }
        else if (k == "map_dudv") {
            std::string line = v.as<std::string>();
            material.map_dudv = resolveTexturePath(line);
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
    std::vector<double> a;
    for (const auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec4 SceneFile::readRGBA(const YAML::Node& node) {
    std::vector<double> a;
    for (const auto& e : node) {
        a.push_back(e.as<double>());
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
        a.push_back(e.as<double>());
    }
    // NOTE KI check if just single number
    if (a.size() < 1) {
        a.push_back(1.0);
    }
    return glm::vec2{ a[0], a[1] };
}

const std::string SceneFile::resolveTexturePath(const std::string& line)
{
    std::string k;
    std::stringstream is2(line);
    is2 >> k;
    std::stringstream tmp;
    tmp << is2.rdbuf();
    std::string path = tmp.str();
    path.erase(0, path.find_first_not_of(' '));
    return path;
}
