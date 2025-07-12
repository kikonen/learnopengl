#pragma once

#include <string>
#include <map>

#include <glm/glm.hpp>

#include "ki/yaml.h"

namespace assets
{
    std::string readString(const YAML::Node& node);
    bool readBool(const YAML::Node& node);
    int readInt(const YAML::Node& node);
    float readFloat(const YAML::Node& node);

    std::vector<int> readIntVector(const YAML::Node& node, int reserve);
    std::vector<float> readFloatVector(const YAML::Node& node, int reserve);

    glm::uvec2 readUVec2(const YAML::Node& node);
    glm::uvec3 readUVec3(const YAML::Node& node);

    glm::vec2 readVec2(const YAML::Node& node);
    glm::vec3 readVec3(const YAML::Node& node);
    glm::vec4 readVec4(const YAML::Node& node);

    glm::vec2 readScale2(const YAML::Node& node);

    glm::vec3 readRGB(const YAML::Node& node);
    glm::vec4 readRGBA(const YAML::Node& node);

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const YAML::Node& v);

    std::string renderNode(
        const YAML::Node& node);

}
