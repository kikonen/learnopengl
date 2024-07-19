#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "loader/document.h"

#include "BaseData.h"

#include "BaseId.h"
#include "BaseData.h"

struct Material;

namespace loader {
    const std::string ROOT_ID{ "ROOT" };

    struct MaterialData;

    void loadRepeat(
        const loader::DocNode& node,
        Repeat& data);

    void loadTiling(
        const loader::DocNode& node,
        Tiling& data);

    const std::string& readString(const loader::DocNode& node);
    bool readBool(const loader::DocNode& node);
    int readInt(const loader::DocNode& node);
    float readFloat(const loader::DocNode& node);

    std::vector<std::string> readStringVector(const loader::DocNode& node, int reserve);
    std::vector<int> readIntVector(const loader::DocNode& node, int reserve);
    std::vector<float> readFloatVector(const loader::DocNode& node, int reserve);

    glm::vec2 readVec2(const loader::DocNode& node);
    glm::vec3 readVec3(const loader::DocNode& node);
    glm::vec4 readVec4(const loader::DocNode& node);

    glm::uvec3 readUVec3(const loader::DocNode& node);

    glm::vec3 readScale3(const loader::DocNode& node);

    glm::vec3 readRGB(const loader::DocNode& node);
    glm::vec4 readRGBA(const loader::DocNode& node);

    // SPEC: [w, x, y, z]
    glm::quat readQuat(const loader::DocNode& node);

    glm::vec3 readDegreesRotation(const loader::DocNode& node);

    float readFractional(const loader::DocNode& node);
    glm::vec2 readRefractionRatio(const loader::DocNode& node);
    float convertRefractionRatio(const glm::vec2 ratio);

    std::tuple<ki::node_id, std::string> resolveId(
        const BaseId& baseId,
        const int cloneIndex,
        const glm::uvec3& tile,
        bool automatic);

    std::string expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile,
        bool automatic);

    BaseId readId(const loader::DocNode& node);

    std::string readFile(std::string_view filename);

    std::string resolveTexturePath(
        std::string_view line,
        bool useCompressed);

    const Material* findMaterial(
        std::string_view name,
        const std::vector<MaterialData>& materials);

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const loader::DocNode&);

    std::string renderNode(
        const loader::DocNode& v);
}
