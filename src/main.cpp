#include <iostream>
#include <fstream>

#include <entt/entt.hpp>
#include <yaml-cpp/yaml.h>

#include "Engine.h"
#include "Test6.h"

void testYAML() {
    std::ifstream fin("scene/scene_full.yml");

    YAML::Node doc = YAML::Load(fin);
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

int main()
{
    entt::registry registry;
    Log::init();
    KI_INFO("START");

    testYAML();

    Engine* engine = new Test6();
    Engine::current = engine;

    if (engine->init()) {
        return -1;
    }

    engine->run();

    delete engine;

    KI_INFO("DONE");

    if (false) {
        std::cout << "PRESS [ENTER] TO CLOSE";
        std::cin.get();
    }

    return 0;
}
