#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

//#include <yaml-cpp/node/node.h>

#include "ki/yaml.h"

namespace mesh_set::decoder
{
    std::vector<float> decodeCompressedFloats(const YAML::Node& node);

    std::vector<uint32_t> decodeCompressedUint32(const YAML::Node& node);

    glm::vec2 decodeVec2(const YAML::Node& node);

    glm::vec3 decodeVec3(const YAML::Node& node);

    glm::vec4 decodeVec4(const YAML::Node& node);

    glm::vec4 decodeRGBA(const YAML::Node& node);

    glm::quat decodeQuat(const YAML::Node& node);

    glm::mat3 decodeMat3(const YAML::Node& node);

    glm::mat4 decodeMat4(const YAML::Node& node);

    class Decoder
    {
    public:
        Decoder() = default;
        ~Decoder() = default;
    };
}
