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
        std::string_view filename);
    ~AssetsFile();

    Assets load();

private:
    void resolveDirs(
        Assets& data);

    void loadAssets(
        const YAML::Node& node,
        Assets& data);

    bool readBool(const YAML::Node& node) const;
    int readInt(const YAML::Node& node) const;
    float readFloat(const YAML::Node& node) const;

    std::vector<int> readIntVector(const YAML::Node& node, int reserve) const;
    std::vector<float> readFloatVector(const YAML::Node& node, int reserve) const;

    glm::uvec2 readUVec2(const YAML::Node& node) const;
    glm::uvec3 readUVec3(const YAML::Node& node) const;

    glm::vec2 readVec2(const YAML::Node& node) const;
    glm::vec3 readVec3(const YAML::Node& node) const;
    glm::vec4 readVec4(const YAML::Node& node) const;

    glm::vec2 readScale2(const YAML::Node& node) const;

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const YAML::Node&) const;

    std::string renderNode(
        const YAML::Node& v) const;

private:
    const std::string m_filename;
};

