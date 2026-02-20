#pragma once

#include <string>
#include <vector>
#include <optional>
#include <tuple>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "loader/document.h"

#include "BaseData.h"

#include "BaseId.h"

#include "Repeat.h"
#include "TerrainTiling.h"

namespace model
{
    class NodeType;
}

struct Material;

namespace loader {
    struct Context;

    const std::string ROOT_ID{ "ROOT" };

    struct NodeTypeData;
    struct MaterialData;

    std::pair<std::string, bool> resolveIncludePath(
        const Context& ctx,
        const std::string& currentDir,
        const std::string& filePath);

    void loadRepeat(
        const loader::DocNode& node,
        Repeat& data);

    void loadTerrainTiling(
        const loader::DocNode& node,
        TerrainTiling& data);

    bool readEnabled(
        const std::string& key);

    std::optional<bool> readEnabled(
        const std::string& key,
        std::optional<bool> defaultValue);

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

    glm::uvec2 readUVec2(const loader::DocNode& node);
    glm::uvec3 readUVec3(const loader::DocNode& node);
    glm::uvec4 readUVec4(const loader::DocNode& node);

    glm::vec3 readScale3(const loader::DocNode& node);

    glm::vec3 readRGB(const loader::DocNode& node);
    glm::vec4 readRGBA(const loader::DocNode& node);

    // SPEC: [w, x, y, z]
    glm::quat readQuat(const loader::DocNode& node);

    glm::vec3 readDegreesRotation(const loader::DocNode& node);

    float readFractional(const loader::DocNode& node);
    glm::vec2 readRefractionRatio(const loader::DocNode& node);
    float convertRefractionRatio(const glm::vec2 ratio);

    std::tuple<ki::node_id, std::string> resolveNodeId(
        const BaseId& typeId,
        const BaseId& baseId);

    std::string expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile);

    BaseId readId(const loader::DocNode& node);

    int readLayer(const loader::DocNode& node);

    const model::NodeType* findNodeType(
        BaseId baseId);

    NodeTypeData* findNodeTypeData(
        BaseId baseId,
        std::vector<NodeTypeData>& nodeTypes);

    const NodeTypeData* findNodeTypeData(
        BaseId baseId,
        const std::vector<NodeTypeData>& nodeTypes);

    const Material* findMaterial(
        std::string_view name,
        const std::vector<MaterialData>& materials);

    bool isIgnore(std::string_view k);

    void reportUnknown(
        std::string_view scope,
        std::string_view k,
        const loader::DocNode&);

    std::string renderNode(
        const loader::DocNode& v);

    std::string renderNodes(
        const std::vector<loader::DocNode>& nodes);
}
