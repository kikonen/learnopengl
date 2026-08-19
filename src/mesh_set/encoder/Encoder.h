#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <yaml-cpp/emitter.h>

namespace mesh_set::encoder
{
    void encodeCompressed(
        YAML::Emitter& out,
        std::vector<float> values
    );

    void encodeCompressed(
        YAML::Emitter& out,
        std::vector<uint32_t> values
    );

    void encodeVec2(
        YAML::Emitter& out,
        const glm::vec2& vec
    );

    void encodeVec3(
        YAML::Emitter& out,
        const glm::vec3& vec
    );

    void encodeVec4(
        YAML::Emitter& out,
        const glm::vec4& vec
    );

    void encodeRGBA(
        YAML::Emitter& out,
        const glm::vec4& vec
    );

    void encodeQuat(
        YAML::Emitter& out,
        const glm::quat& quat
    );

    void encodeMat3(
        YAML::Emitter& out,
        const glm::mat3& mat
    );

    void encodeMat4(
        YAML::Emitter& out,
        const glm::mat4& mat
    );

    void encodeVec2(
        std::vector<float>& out,
        const glm::vec2& vec
    );

    void encodeVec3(
        std::vector<float>& out,
        const glm::vec3& vec
    );

    void encodeVec4(
        std::vector<float>& out,
        const glm::vec4& vec
    );

    void encodeQuat(
        std::vector<float>& out,
        const glm::quat& quat
    );

    class Encoder
    {
    public:
        Encoder() = default;
        ~Encoder() = default;
    };
}
