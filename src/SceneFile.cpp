#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "asset/MeshLoader.h"

#include "SceneFile.h"

namespace {
    const double DEF_ALPHA = 1.0;
}

SceneFile::SceneFile(
    std::shared_ptr<AsyncLoader> asyncLoader,
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

std::shared_ptr<Scene> SceneFile::load(std::shared_ptr<Scene> scene)
{
    std::ifstream fin(filename);
    YAML::Node doc = YAML::Load(fin);

    loadSkybox(doc, skybox, materials);
    loadMaterials(doc, materials);
    loadEntities(doc, entities, materials);

    attach(scene, skybox, entities, materials);

    return scene;
}

void SceneFile::attach(
    std::shared_ptr<Scene> scene,
    SkyboxData& skybox,
    std::map<const uuids::uuid, EntityData>& entities,
    std::vector<std::shared_ptr<Material>>& materials)
{
    attachSkybox(scene, skybox, materials);

    for (auto& entry : entities) {
        attachEntity(scene, entry.second, entities, materials);
    }
}

void SceneFile::attachSkybox(
    std::shared_ptr<Scene> scene,
    SkyboxData& data,
    std::vector<std::shared_ptr<Material>>& materials)
{
    if (!skybox.valid()) return;

    auto skybox = std::make_unique<SkyboxRenderer>(assets, data.shaderName, data.materialName);
    skybox->prepare(asyncLoader->shaders);
    scene->skyboxRenderer.reset(skybox.release());
}

void SceneFile::attachEntity(
    std::shared_ptr<Scene> scene,
    const EntityData& data,
    std::map<const uuids::uuid, EntityData>& entities,
    std::vector<std::shared_ptr<Material>>& materials)
{
    asyncLoader->addLoader([this, scene, &data, &entities, &materials]() {
        if (!data.enabled) {
            return;
        }

        const EntityData* parent = nullptr;
        if (!data.parentId.is_nil()) {
            auto entry = entities.find(data.parentId);
            if (entry != entities.end()) {
                auto& e = entry->second;
                parent = &e;
            }
        }

        auto type = std::make_shared<NodeType>(data.typeId, asyncLoader->getShader(data.shaderName, data.shaderDefinitions));

        {
            auto e = data.renderFlags.find("blend");
            if (e != data.renderFlags.end()) {
                type->blend = e->second;
            }
        }
        {
            auto e = data.renderFlags.find("render_back");
            if (e != data.renderFlags.end()) {
                type->renderBack = e->second;
            }
        }
        {
            auto e = data.renderFlags.find("no_shadow");
            if (e != data.renderFlags.end()) {
                type->noShadow = e->second;
            }
        }
        {
            auto e = data.renderFlags.find("mirror");
            if (e != data.renderFlags.end()) {
                type->mirror = e->second;
                type->mirrorPlane = data.mirrorPlane;
            }
        }
        {
            auto e = data.renderFlags.find("water");
            if (e != data.renderFlags.end()) {
                type->water = e->second;
            }
        }
        {
            auto e = data.renderFlags.find("light");
            if (e != data.renderFlags.end()) {
                type->light = e->second;
            }
        }
        {
            auto e = data.renderFlags.find("batch_mode");
            if (e != data.renderFlags.end()) {
                type->batchMode = e->second;
            }
        }
        {
            auto e = data.renderFlags.find("wireframe");
            if (e != data.renderFlags.end()) {
                type->wireframe = e->second;
            }
        }

        {
            MeshLoader meshLoader(assets, data.modelName, data.modelPath);

            if (data.defaultMaterial) {
                meshLoader.defaultMaterial = data.defaultMaterial;
                meshLoader.overrideMaterials = data.overrideMaterials;
            }
            meshLoader.loadTextures = data.loadTextures;

            auto mesh = meshLoader.load();
            KI_INFO_SB("SCENE_FILE ATTACH: type=" << type->typeID << ", mesh=" << mesh->modelName);
            type->mesh.reset(mesh.release());
        }

        type->modifyMaterials([this, &data](Material& m) {
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
                m.loadTextures(asyncLoader->assets);
            }
         });

        auto& repeat = data.repeat;
        for (auto z = 0; z < repeat.zCount; z++) {
            for (auto y = 0; y < repeat.yCount; y++) {
                for (auto x = 0; x < repeat.xCount; x++) {
                    for (auto& p : data.positions) {
                        glm::vec3 pos = p;
                        if (parent) {
                            pos += parent->positions[0];
                        }
                        pos += glm::vec3{ x * repeat.xStep, y * repeat.yStep, z * repeat.zStep };

                        auto node = new Node(type);
                        node->id = data.id;
                        node->parentId = data.parentId;

                        node->setPos(pos + assets.groundOffset);
                        node->setRotation(data.rotation);
                        node->setScale(data.scale);

                        node->selected = data.selected;

                        scene->registry.addNode(node);
                    }
                }
            }
        }
        });
}

void SceneFile::loadSkybox(
    const YAML::Node& doc,
    SkyboxData& data,
    std::vector<std::shared_ptr<Material>>& materials)
{
    auto& node = doc["skybox"];

    if (!node) return;

    for (auto pair : node) {
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
    std::vector<std::shared_ptr<Material>>& materials)
{
    for (auto& entry : doc["entities"]) {
        EntityData data;
        loadEntity(entry, materials, data);
        // NOTE KI ignore elements without ID
        if (data.id.is_nil()) continue;
        entities[data.id] = data;
    }
}

void SceneFile::loadEntity(
    const YAML::Node& node,
    std::vector<std::shared_ptr<Material>>& materials,
    EntityData& data)
{
    for (auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        //std::cout << k << " = " << v << "\n";

        if (k == "type") {
            std::string type = v.as<std::string>();
            if (type == "node") {
                data.type = EntityType::node;
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
                for (auto& name : v) {
                    data.shaderDefinitions.push_back(name.as<std::string>());
                }
            }
        }
        else if (k == "render_flags") {
            if (v.Type() == YAML::NodeType::Sequence) {
                for (auto& name : v) {
                    auto flag = name.as<std::string>();
                    data.renderFlags[flag] = true;
                }
            }
        }
        else if (k == "mirror_plane") {
            data.mirrorPlane = readVec4(v);
        }
        else if (k == "default_material") {
            const std::string materialName = v.as<std::string>();
            auto material = Material::find(materialName, materials);
            if (material) {
                // NOTE KI need to create copy *IF* modifiers
                // TODO KI should make copy *ALWAYS* for safety
                data.defaultMaterial = data.materialModifierFields.any()
                    ? std::make_shared<Material>(*material)
                    : material;
            }
        }
        else if (k == "material_modifier") {
            loadMaterialModifiers(v, data);
        }
        else if (k == "override_material") {
            data.overrideMaterials = v.as<bool>();
        }
        else if (k == "load_textures") {
            data.loadTextures = v.as<bool>();
        }
        else if (k == "pos") {
            data.positions.push_back(readVec3(v));
        }
        else if (k == "positions") {
            data.positions.clear();
            for (auto& p : v) {
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
            loadRepeat(v, data);
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
        data.positions.emplace_back(0);
    }
}

void SceneFile::loadMaterialModifiers(
    const YAML::Node& node,
    EntityData& data)
{
    data.materialModifiers = std::make_shared<Material>();
    data.materialModifiers->name = "<modifier>";

    loadMaterial(node, data.materialModifierFields, data.materialModifiers);
}

void SceneFile::loadRepeat(const YAML::Node& node, EntityData& data)
{
    auto& repeat = data.repeat;

    for (auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "x_count") {
            repeat.xCount = v.as<int>();
        }
        else if (k == "y_count") {
            repeat.yCount = v.as<int>();
        }
        else if (k == "z_count") {
            repeat.zCount = v.as<int>();
        }
        else if (k == "x_step") {
            repeat.xStep = v.as<double>();
        }
        else if (k == "y_step") {
            repeat.yStep = v.as<double>();
        }
        else if (k == "z_step") {
            repeat.zStep = v.as<double>();
        }
        else {
            std::cout << "UNKNOWN REPEAT_ENTRY: " << k << "=" << v << "\n";
        }
    }
}

void SceneFile::loadMaterials(
    const YAML::Node& doc,
    std::vector<std::shared_ptr<Material>>& materials) {
    for (auto& entry : doc["materials"]) {
        MaterialField fields;
        std::shared_ptr<Material> material{ nullptr };

        loadMaterial(entry, fields, material);
        if (material) {
            materials.push_back(material);
        }
    }
}

void SceneFile::loadMaterial(
    const YAML::Node& node,
    MaterialField& fields,
    std::shared_ptr<Material>& material)
{
    for (auto& pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "name") {
            const std::string& name = v.as<std::string>();
            material = std::make_shared<Material>();
            material->name = name;
        }
        else if (material) {
            if (k == "type") {
                std::string type = v.as<std::string>();
                if (type == "model") {
                    material->type = MaterialType::model;
                }
                else if (type == "texture") {
                    material->type = MaterialType::texture;
                }
                else if (type == "sprite") {
                    material->type = MaterialType::sprite;
                }
                else {
                    std::cout << "UNKNOWN MATERIAL_TYPE: " << k << "=" << v << "\n";
                }
            }
            else if (k == "ns") {
                material->ns = v.as<float>();
            }
            else if (k == "ka") {
                material->ka = readRGBA(v);
            }
            else if (k == "kd") {
                material->kd = readRGBA(v);
            }
            else if (k == "ks") {
                material->ks = readRGBA(v);
            }
            else if (k == "ke") {
                material->ke = readRGBA(v);
            }
            else if (k == "ni") {
                material->ni = v.as<float>();
            }
            else if (k == "d") {
                material->d = v.as<float>();
            }
            else if (k == "illum") {
                material->d = v.as<float>();
            }
            else if (k == "map_kd") {
                std::string line = v.as<std::string>();
                material->map_kd = resolveTexturePath(line);
            }
            else if (k == "map_ke") {
                std::string line = v.as<std::string>();
                material->map_ke = resolveTexturePath(line);
            }
            else if (k == "map_ks") {
                std::string line = v.as<std::string>();
                material->map_ks = resolveTexturePath(line);
            }
            else if (k == "map_bump") {
                std::string line = v.as<std::string>();
                material->map_bump = resolveTexturePath(line);
            }
            else if (k == "bump") {
                std::string line = v.as<std::string>();
                material->map_bump = resolveTexturePath(line);
            }
            else if (k == "reflection") {
                material->reflection = v.as<float>();
                fields.reflection = true;
            }
            else if (k == "refraction") {
                material->refraction = v.as<float>();
                fields.refraction = true;
            }
            else if (k == "refraction_ratio") {
                material->refractionRatio = readRefractionRatio(v);
                fields.refractionRatio = true;
            }
            else if (k == "fog_ratio") {
                material->fogRatio = v.as<float>();
            }
            else if (k == "tiling") {
                material->tiling = v.as<float>();
                fields.tiling = true;
            }
            else if (k == "texture_spec") {
                loadTextureSpec(v, material->textureSpec);
                fields.textureSpec = true;
            }
            else {
                std::cout << "UNKNOWN MATERIAL_ENTRY: " << k << "=" << v << "\n";
            }
        }
    }
}

void SceneFile::loadTextureSpec(
    const YAML::Node& node,
    TextureSpec& textureSpec)
{
    for (auto& pair : node) {
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
    for (auto& e : node) {
        a.push_back(e.as<float>());
    }
    return glm::vec2{ a[0], a[1] };
}

glm::vec3 SceneFile::readVec3(const YAML::Node& node) {
    std::vector<double> a;
    for (auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 SceneFile::readVec4(const YAML::Node& node) {
    std::vector<double> a;
    for (auto& e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

glm::vec4 SceneFile::readRGBA(const YAML::Node& node) {
    std::vector<double> a;
    for (auto& e : node) {
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
    for (auto& e : node) {
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
