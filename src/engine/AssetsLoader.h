#pragma once

#include <string>
#include <map>

#include "ki/yaml.h"

#include "asset/Assets.h"

class AssetsLoader final
{
public:
    AssetsLoader(
        std::string_view filename);
    ~AssetsLoader();

    Assets load();

private:
    void resolveDirs(
        Assets& data);

    void loadAssets(
        const YAML::Node& node,
        Assets& data);

    void loadLayers(
        const YAML::Node& node,
        std::vector<LayerInfo>& layers);

    void loadLayer(
        const YAML::Node& node,
        LayerInfo& layer);

    std::string readString(const YAML::Node& node) const;
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

    ViewportEffect readViewportEffect(
        const std::string& key,
        const YAML::Node& node) const;

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const YAML::Node&) const;

    std::string renderNode(
        const YAML::Node& v) const;

private:
    const std::string m_filename;
};

