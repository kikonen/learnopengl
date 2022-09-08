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
    const Assets& assets,
    const std::string& filename)
    : filename(filename),
    assets(assets),
    loader(assets)
{
}

SceneFile::~SceneFile()
{
}

std::shared_ptr<Scene> SceneFile::load(std::shared_ptr<Scene> scene)
{
    loader.scene = scene;

    std::ifstream fin(filename);
    YAML::Node doc = YAML::Load(fin);

    SkyboxData skybox;
    std::map<const uuids::uuid, EntityData> entities;
    std::map<const std::string, std::shared_ptr<Material>> materials;

    loadSkybox(doc, skybox, materials);
    loadMaterials(doc, materials);
    loadEntities(doc, entities, materials);

    attach(skybox, entities, materials);

    return scene;
}

void SceneFile::attach(
    SkyboxData& skybox,
    std::map<const uuids::uuid, EntityData>& entities,
    std::map<const std::string, std::shared_ptr<Material>>& materials)
{
    attachSkybox(skybox, materials);

    for (auto entry : entities) {
        attachEntity(entry.second, entities, materials);
    }
}

void SceneFile::attachSkybox(
    SkyboxData& data,
    std::map<const std::string, std::shared_ptr<Material>>& materials)
{
    auto scene = loader.scene;
    auto skybox = new SkyboxRenderer(assets, data.shaderName, data.materialName);
    skybox->prepare(scene->shaders);
    scene->skyboxRenderer.reset(skybox);
}

void SceneFile::attachEntity(
    const EntityData& data,
    std::map<const uuids::uuid, EntityData>& entities,
    std::map<const std::string, std::shared_ptr<Material>>& materials)
{
    loader.addLoader([this, data, entities, materials]() {
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

        auto type = std::make_shared<NodeType>(data.typeId, loader.getShader(data.shaderName, data.shaderDefinitions));

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

        MeshLoader meshLoader(assets, data.modelName, data.modelPath);

        if (data.defaultMaterial) {
            meshLoader.defaultMaterial = data.defaultMaterial;
            meshLoader.overrideMaterials = data.overrideMaterials;
        }

        type->mesh = meshLoader.load();

        type->modifyMaterials([&data](Material& m) {
            if (data.materialModifierFields.reflection) {
                m.reflection = data.materialModifiers->reflection;
            }
            if (data.materialModifierFields.refraction) {
                m.refraction = data.materialModifiers->refraction;
            }
            if (data.materialModifierFields.refractionRatio) {
                m.refractionRatio = data.materialModifiers->refractionRatio;
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

                        loader.scene->registry.addNode(node);
                    }
                }
            }
        }
        });
}

void SceneFile::loadSkybox(
    const YAML::Node& doc,
    SkyboxData& data,
    std::map<const std::string, std::shared_ptr<Material>>& materials)
{
    auto node = doc["skybox"];

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
    std::map<const std::string, std::shared_ptr<Material>>& materials)
{
    for (auto entry : doc["entities"]) {
        EntityData data;
        loadEntity(entry, materials, data);
        // NOTE KI ignore elements without ID
        if (data.id.is_nil()) continue;
        entities[data.id] = data;
    }
}

void SceneFile::loadEntity(
    const YAML::Node& node,
    std::map<const std::string, std::shared_ptr<Material>>& materials,
    EntityData& data)
{
    for (auto pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        //std::cout << k << " = " << v << "\n";

        if (k == "name") {
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
                for (auto name : v) {
                    data.shaderDefinitions.push_back(name.as<std::string>());
                }
            }
        }
        else if (k == "render_flags") {
            if (v.Type() == YAML::NodeType::Sequence) {
                for (auto name : v) {
                    auto flag = name.as<std::string>();
                    data.renderFlags[flag] = true;
                }
            }
        }
        else if (k == "mirror_plane") {
            data.mirrorPlane = readVec4(v);
        }
        else if (k == "default_material") {
            std::string materialName = v.as<std::string>();
            auto entry = materials.find(materialName);
            if (entry != materials.end()) {
                // NOTE KI need to create copy *IF* modifiers
                data.defaultMaterial = data.materialModifierFields.any()
                    ? std::make_shared<Material>(*entry->second)
                    : entry->second;
            }
        }
        else if (k == "material_modifier") {
            loadMaterialModifiers(v, data);
        }
        else if (k == "override_material") {
            data.overrideMaterials = v.as<bool>();
        }
        else if (k == "pos") {
            data.positions.push_back(readVec3(v));
        }
        else if (k == "positions") {
            data.positions.clear();
            for (auto p : v) {
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
    const std::string materialPath = assets.modelsDir;
    const std::string& name{ "override_material" };
    data.materialModifiers = std::make_shared<Material>(name, materialPath);

    loadMaterial(node, data.materialModifierFields, data.materialModifiers);
}

void SceneFile::loadRepeat(const YAML::Node& node, EntityData& data)
{
    auto& repeat = data.repeat;

    for (auto pair : node) {
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
    std::map<const std::string, std::shared_ptr<Material>>& materials) {
    for (auto entry : doc["materials"]) {
        MaterialField fields;
        std::shared_ptr<Material> material{ nullptr };

        loadMaterial(entry, fields, material);
        if (material) {
            materials[material->name] = material;
        }
    }
}

void SceneFile::loadMaterial(
    const YAML::Node& node,
    MaterialField& fields,
    std::shared_ptr<Material>& material)
{
    const std::string materialPath = assets.modelsDir;

    for (auto pair : node) {
        const std::string& k = pair.first.as<std::string>();
        const YAML::Node& v = pair.second;

        if (k == "name") {
            const std::string& name = v.as<std::string>();
            material = std::make_shared<Material>(name, materialPath);
        }
        else if (material) {
            if (k == "ns") {
                material->ns = v.as<double>();
            }
            else if (k == "ka") {
                material->ka = readVec4(v);
            }
            else if (k == "kd") {
                material->kd = readVec4(v);
            }
            else if (k == "ks") {
                material->ks = readVec4(v);
            }
            else if (k == "ke") {
                material->ke = readVec4(v);
            }
            else if (k == "ni") {
                material->ni = v.as<double>();
            }
            else if (k == "d") {
                material->d = v.as<double>();
            }
            else if (k == "illum") {
                material->d = v.as<double>();
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
                material->reflection = v.as<double>();
                fields.reflection = true;
            }
            else if (k == "refraction") {
                material->refraction = v.as<double>();
                fields.refraction = true;
            }
            else if (k == "refraction_ratio") {
                material->refractionRatio = readRefractionRatio(v);
                fields.refractionRatio = true;
            }
            else if (k == "fog_ratio") {
                material->fogRatio = v.as<double>();
            }
            else if (k == "tiling") {
                material->tiling = v.as<double>();
            }
            else {
                std::cout << "UNKNOWN MATERIAL_ENTRY: " << k << "=" << v << "\n";
            }
        }
    }
}

void SceneFile::testYAML() {
    std::ifstream fin(filename);

    YAML::Node doc = YAML::Load(fin);
    if (!doc["name"]) {
        std::cerr << "FAILED to load " << filename;
        return;
    }

    std::string name = doc["name"].as<std::string>();
    std::cout << name << "\n";

    const YAML::Node& entities = doc["entities"];
    std::cout << entities.size() << "\n";
    std::cout << "---------------\n";

    for (auto node : entities) {
        for (auto pair : node) {
            std::string k = pair.first.as<std::string>();
            auto v = pair.second;
            switch (node.Type()) {
            case YAML::NodeType::Null:
                std::cout << "Type: null\n";
                break;
            case YAML::NodeType::Scalar:
                std::cout << "Type: scalar\n";
                break;
            case YAML::NodeType::Sequence:
                std::cout << "Type: sequence\n";
                break;
            case YAML::NodeType::Map:
                std::cout << "Type: map\n";
                break;
            case YAML::NodeType::Undefined:
                std::cout << "Type: undefined\n";
                break;
            }
            std::cout << "key: " << k << "\n";
            std::cout << "Val: { " << v << " }\n";
        }
        std::cout << "---------------\n";
        //        std::string name = node["name"].as<std::string>();
    //        std::cout << name << "\n";
    }
}

glm::vec3 SceneFile::readVec3(const YAML::Node& node) {
    std::vector<double> a;
    for (auto e : node) {
        a.push_back(e.as<double>());
    }
    return glm::vec3{ a[0], a[1], a[2] };
}

glm::vec4 SceneFile::readVec4(const YAML::Node& node) {
    std::vector<double> a;
    for (auto e : node) {
        a.push_back(e.as<double>());
    }
    // NOTE KI check if alpha is missing
    if (a.size() < 4) {
        a.push_back(DEF_ALPHA);
    }
    return glm::vec4{ a[0], a[1], a[2], a[3] };
}

double SceneFile::readRefractionRatio(const YAML::Node& node) {
    std::vector<double> a;
    for (auto e : node) {
        a.push_back(e.as<double>());
    }
    // NOTE KI check if just single number
    if (a.size() < 1) {
        a.push_back(1.0);
    }
    return a[0] / a[1];
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
