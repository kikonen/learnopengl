#pragma once

#include <string>
#include <map>

#include <stduuid/uuid.h>
#include <yaml-cpp/yaml.h>

#include "asset/Assets.h"

class AssetsFile final
{
public:
    AssetsFile(
        const std::string& filename);
    ~AssetsFile();

    Assets load();

private:
    void loadAssets(
        const YAML::Node& node,
        Assets& data);

    glm::vec2 readVec2(const YAML::Node& node);
    glm::vec3 readVec3(const YAML::Node& node);
    glm::vec4 readVec4(const YAML::Node& node);

private:
    const std::string filename;
};

