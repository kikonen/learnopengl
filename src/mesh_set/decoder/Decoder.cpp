#include "Decoder.h"

#include <cstring>

#include "util/compress.h"

namespace mesh_set::decoder
{
    std::vector<float> decodeCompressedFloats(const YAML::Node& node)
    {
        if (!node) return {};
        if (node.IsNull()) return {};

        try {
            const auto& binary = node.as<std::string>();
            const auto& decompressed = util::decompress_zlib(
                binary.data(),
                binary.size(),
                0
            );
            const size_t count = decompressed.size() / sizeof(float);
            std::vector<float> values(count);
            std::memcpy(values.data(), decompressed.data(), decompressed.size());
            return values;
        }
        catch (...) {
            return {};
        }
    }

    std::vector<uint32_t> decodeCompressedUint32(const YAML::Node& node)
    {
        if (!node) return {};
        if (node.IsNull()) return {};

        try {
            const auto& binary = node.as<std::string>();
            const auto& decompressed = util::decompress_zlib(
                binary.data(),
                binary.size(),
                0
            );
            const size_t count = decompressed.size() / sizeof(uint32_t);
            std::vector<uint32_t> values(count);
            std::memcpy(values.data(), decompressed.data(), decompressed.size());
            return values;
        }
        catch (...) {
            return {};
        }
    }

    glm::vec2 decodeVec2(const YAML::Node& node)
    {
        if (!node || !node.IsSequence() || node.size() < 2) return glm::vec2{0.0f};
        return glm::vec2{node[0].as<float>(), node[1].as<float>()};
    }

    glm::vec3 decodeVec3(const YAML::Node& node)
    {
        if (!node || !node.IsSequence() || node.size() < 3) return glm::vec3{0.0f};
        return glm::vec3{node[0].as<float>(), node[1].as<float>(), node[2].as<float>()};
    }

    glm::vec4 decodeVec4(const YAML::Node& node)
    {
        if (!node || !node.IsSequence() || node.size() < 4) return glm::vec4{0.0f};
        return glm::vec4{node[0].as<float>(), node[1].as<float>(), node[2].as<float>(), node[3].as<float>()};
    }

    glm::vec4 decodeRGBA(const YAML::Node& node)
    {
        return decodeVec4(node);
    }

    glm::quat decodeQuat(const YAML::Node& node)
    {
        if (!node || !node.IsSequence() || node.size() < 4) return glm::quat{1.0f, 0.0f, 0.0f, 0.0f};
        return glm::quat{node[3].as<float>(), node[0].as<float>(), node[1].as<float>(), node[2].as<float>()};
    }

    glm::mat3 decodeMat3(const YAML::Node& node)
    {
        glm::mat3 mat{1.0f};
        if (!node || !node.IsSequence()) return mat;
        for (size_t i = 0; i < 3 && i < node.size(); ++i) {
            const auto& row = node[i];
            if (row.IsSequence() && row.size() >= 3) {
                mat[i][0] = row[0].as<float>();
                mat[i][1] = row[1].as<float>();
                mat[i][2] = row[2].as<float>();
            }
        }
        return mat;
    }

    glm::mat4 decodeMat4(const YAML::Node& node)
    {
        glm::mat4 mat{1.0f};
        if (!node || !node.IsSequence()) return mat;
        for (size_t i = 0; i < 4 && i < node.size(); ++i) {
            const auto& row = node[i];
            if (row.IsSequence() && row.size() >= 4) {
                mat[i][0] = row[0].as<float>();
                mat[i][1] = row[1].as<float>();
                mat[i][2] = row[2].as<float>();
                mat[i][3] = row[3].as<float>();
            }
        }
        return mat;
    }
}
