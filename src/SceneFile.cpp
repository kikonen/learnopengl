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

    testYAML();

    return scene;
}

void SceneFile::loadEntities(const YAML::Node& doc) {
	const YAML::Node& entries = doc["entities"];

	for (auto entry : entries) {
		int typeId { 0 };
		std::string name {};
		std::string model {};
		std::string path { "/" };
		std::string material {};
		std::string shader { TEX_TEXTURE };
		glm::vec3 pos { 0 };
		double scale { 1 };

		for (auto pair : entry) {
			const std::string& k = pair.first.as<std::string>();
			const YAML::Node& v = pair.second;

			std::cout << k << " = " << v << "\n";

			if (k == "name") {
				name = v.as<std::string>();
			}
			else if (k == "type_id") {
				typeId = v.as<int>();
			}
			else if (k == "model") {
				if (v.Type() == YAML::NodeType::Sequence) {
					model = v[0].as<std::string>();
					path = v[1].as<std::string>();
				}
				else {
					model = v.as<std::string>();
				}
			}
			else if (k == "shader") {
				shader = v.as<std::string>();
				if (shader == "texture") {
					shader = TEX_TEXTURE;
				}
			}
			else if (k == "material") {
				material = v.as<std::string>();
			}
			else if (k == "pos") {
				pos = readVec3(v);
			}
			else if (k == "scale") {
				scale = v.as<double>();
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
*/
		loader.addLoader([this, typeId, shader, model, pos, material, path, scale]() {
			NodeType* type = new NodeType(typeId, loader.getShader(shader));
			MeshLoader meshLoader(assets, model, path);

			if (material != "") {
				if (materials.count(material)) {
					meshLoader.defaultMaterial = materials[material];
					meshLoader.overrideMaterials = true;
				}
			}

			type->mesh = meshLoader.load();

			if (material != "") {
				//type->modifyMaterials([](Material& m) { m.reflection = 0.05f; });
			}

			Node* node = new Node(type);
			node->setPos(pos + assets.groundOffset);
			node->setScale(scale);

			loader.scene->registry.addNode(node);
		});
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
