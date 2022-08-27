#include <iostream>
#include <fstream>

#include <yaml-cpp/yaml.h>

#include "SceneFile.h"


SceneFile::SceneFile(
    const Assets& assets,
    const std::string& filename)
	: filename(filename), assets(assets)
{
}

SceneFile::~SceneFile()
{
}

Scene* SceneFile::load()
{
    AsyncLoader loader(assets);
    testYAML();
    return nullptr;
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
