#pragma once

#include <string>
#include <map>

#include <ki/uuid.h>

#pragma warning(push)
#pragma warning(disable : 4251)
#pragma warning(disable : 4275)

#include <yaml-cpp/yaml.h>

#pragma warning(pop)


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

    glm::vec2 readVec2(const YAML::Node& node) const;
    glm::vec3 readVec3(const YAML::Node& node) const;
    glm::vec4 readVec4(const YAML::Node& node) const;

    glm::vec2 readScale2(const YAML::Node& node) const;

    void reportUnknown(
        const std::string& scope,
        const std::string& k,
        const YAML::Node&) const;

private:
    const std::string m_filename;
};

