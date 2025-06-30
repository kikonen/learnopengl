#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "loader/document.h"

#include "BaseData.h"

#include "BaseId.h"

#include "Repeat.h"
#include "TerrainTiling.h"

struct Material;

class NodeType;

namespace loader {
    const std::string ROOT_ID{ "ROOT" };

    struct NodeTypeData;
    struct MaterialData;

    void loadRepeat(
        const loader::DocNode& node,
        Repeat& data);

    void loadTerrainTiling(
        const loader::DocNode& node,
        TerrainTiling& data);

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

    std::tuple<ki::node_id, std::string> resolveNodeId(
        const BaseId& typeId,
        const BaseId& baseId);

    std::string expandMacros(
        const std::string& str,
        const int cloneIndex,
        const glm::uvec3& tile);

    BaseId readId(const loader::DocNode& node);

    int readLayer(const loader::DocNode& node);

    const NodeType* findNodeType(
        BaseId baseId);

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
