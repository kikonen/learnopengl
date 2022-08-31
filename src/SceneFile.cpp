#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include "asset/MeshLoader.h"

#include "SceneFile.h"

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

Scene* SceneFile::load(Scene* scene)
{
	if (!scene) {
		scene = new Scene(assets);
	}
	loader.scene = scene;

    std::ifstream fin(filename);
    YAML::Node doc = YAML::Load(fin);

    loadMaterials(doc);
    loadEntities(doc);

//    testYAML();

    return scene;
}

void SceneFile::loadEntities(const YAML::Node& doc) {
	const YAML::Node& entries = doc["entities"];

	for (auto entry : entries) {
		int typeId { 0 };
		std::string name {};
		std::string modelName {};
		std::string modelPath { "/" };
		std::string materialName {};
		std::string shaderName { TEX_TEXTURE };
		std::vector<std::string> shaderDefinitions {};
		std::map<std::string, bool> renderFlags {};
		glm::vec3 pos { 0 };
		glm::vec3 rotation{ 0 };
		glm::vec4 mirrorPlane{ 0 };
		double scale { 1 };

		for (auto pair : entry) {
			const std::string& k = pair.first.as<std::string>();
			const YAML::Node& v = pair.second;

			//std::cout << k << " = " << v << "\n";

			if (k == "name") {
				name = v.as<std::string>();
			}
			else if (k == "type_id") {
				typeId = v.as<int>();
			}
			else if (k == "model") {
				if (v.Type() == YAML::NodeType::Sequence) {
					modelName = v[0].as<std::string>();
					modelPath = v[1].as<std::string>();
				}
				else {
					modelName = v.as<std::string>();
				}
			}
			else if (k == "shader") {
				shaderName = v.as<std::string>();
				if (shaderName == "texture") {
					shaderName = TEX_TEXTURE;
				}
			}
			else if (k == "shader_definitions") {
				if (v.Type() == YAML::NodeType::Sequence) {
					for (auto name : v) {
						shaderDefinitions.push_back(name.as<std::string>());
					}
				}
			}
			else if (k == "render_flags") {
				if (v.Type() == YAML::NodeType::Sequence) {
					for (auto name : v) {
						auto flag = name.as<std::string>();
						renderFlags[flag] = true;
					}
				}
			}
			else if (k == "mirror_plane") {
				mirrorPlane = readVec4(v);
			}
			else if (k == "material") {
				materialName = v.as<std::string>();
			}
			else if (k == "pos") {
				pos = readVec3(v);
			}
			else if (k == "rotation") {
				rotation = readVec3(v);
			}
			else if (k == "scale") {
				scale = v.as<double>();
			}
			else {
				std::cout << "UNKNOWN: " << k << "\n";
			}
		}

		/*
void SceneLoaderTest::setupNodeMaterialBalls()
{
	addLoader([this]() {
		MaterialType materialTypes[4] = { MaterialType::basic, MaterialType::gold, MaterialType::silver, MaterialType::bronze };

		int index = 0;
		for (auto mt : materialTypes) {
			NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE));

			MeshLoader loader(assets, "water_ball");
			loader.defaultMaterial = Material::createMaterial(mt);
			loader.overrideMaterials = true;
			type->mesh = loader.load();
			type->modifyMaterials([](Material& m) { m.reflection = 0.05f; });

			Node* node = new Node(type);
			node->setPos(glm::vec3(5, 25, 5 + index * 5) + assets.groundOffset);

			scene->registry.addNode(node);
			index++;
		}
	});
}
void SceneLoaderTest::setupNodeWindow1()
{
	addLoader([this]() {
		NodeType* type = new NodeType(NodeType::nextID(), getShader(TEX_TEXTURE, { DEF_USE_ALPHA }));
		type->blend = true;
		type->renderBack = true;

		MeshLoader loader(assets, "window1");
		type->mesh = loader.load();

		Node* node = new Node(type);
		node->setPos(glm::vec3(5, 10, -5) + assets.groundOffset);
		node->setRotation(glm::vec3(0, 180, 0));
		scene->registry.addNode(node);
	});
}*/

//loader.addLoader([this, typeId, shaderName, shaderDefinitions, renderFlags, modelName, modelPath, pos, rotation, scale, materialName]() {
			NodeType* type = new NodeType(typeId, loader.getShader(shaderName, shaderDefinitions));

			{
				std::string flag{ "blend" };
				if (renderFlags.count(flag)) {
					type->blend = renderFlags[flag];
				}
			}
			{
				std::string flag{ "render_back" };
				if (renderFlags.count(flag)) {
					type->renderBack = renderFlags[flag];
				}
			}
			{
				std::string flag{ "no_shadow" };
				if (renderFlags.count(flag)) {
					type->noShadow = renderFlags[flag];
				}
			}
			bool mirror = false;
			{
				std::string flag{ "mirror" };
				if (renderFlags.count(flag)) {
					type->mirror = renderFlags[flag];
				}

				if (renderFlags.count(flag) && renderFlags[flag]) {
					mirror = true;
					type->mirrorPlane = mirrorPlane;
				}
			}
			{
				std::string flag{ "water" };
				if (renderFlags.count(flag)) {
					type->water = renderFlags[flag];
				}
			}
			{
				std::string flag{ "light" };
				if (renderFlags.count(flag)) {
					type->light = renderFlags[flag];
				}
			}
			{
				std::string flag{ "batch_mode" };
				if (renderFlags.count(flag)) {
					type->batchMode = renderFlags[flag];
				}
			}
			{
				std::string flag{ "wireframe" };
				if (renderFlags.count(flag)) {
					type->wireframe = renderFlags[flag];
				}
			}

			MeshLoader meshLoader(assets, modelName, modelPath);

			Material* material = nullptr;
			if (materials.count(materialName)) {
				material = materials[materialName];
			}

			if (material) {
				meshLoader.defaultMaterial = material;
				meshLoader.overrideMaterials = true;
			}

			type->mesh = meshLoader.load();

			if (mirror) {
				type->modifyMaterials([](Material& m) {
					m.reflection = 0.1f;
					m.refraction = 0.9f;
					m.refractionRatio = 1.0f / 1.52;
				});
			}

			Node* node = new Node(type);
			node->setPos(pos + assets.groundOffset);
			node->setRotation(rotation);
			node->setScale(scale);

			loader.scene->registry.addNode(node);
//		});
	}
}

void SceneFile::loadMaterials(const YAML::Node& doc) {
	const std::string materialPath = assets.modelsDir;

	const YAML::Node& entries = doc["materials"];

	for (auto entry : entries) {
		Material* material = nullptr;
		for (auto pair : entry) {
			const std::string& k = pair.first.as<std::string>();
			const YAML::Node& v = pair.second;

			if (k == "name") {
				const std::string& name = v.as<std::string>();
				material = new Material(name, materialPath);
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
				}
				else if (k == "refraction") {
					material->refraction = v.as<double>();
				}
				else if (k == "refraction_ratio") {
					material->refractionRatio = v.as<double>();
				}
				else if (k == "fog_ratio") {
					material->fogRatio = v.as<double>();
				}
				else if (k == "tiling") {
					material->tiling = v.as<double>();
				}
			}
		}

		if (material) {
			materials[material->name] = material;
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

const glm::vec3 SceneFile::readVec3(const YAML::Node& node) {
	std::vector<double> a;
	for (auto e : node) {
		a.push_back(e.as<double>());
	}
	return glm::vec3{ a[0], a[1], a[2] };
}

const glm::vec4 SceneFile::readVec4(const YAML::Node& node) {
	std::vector<double> a;
	for (auto e : node) {
		a.push_back(e.as<double>());
	}
	return glm::vec4{ a[0], a[1], a[2], a[3] };
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
