#pragma once

#include <glm/glm.hpp>

#include <yaml-cpp/emitter.h>

namespace mesh_set::encoder
{
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

    void encodeMat3(
        YAML::Emitter& out,
        const glm::mat3& mat
    );

    void encodeMat4(
        YAML::Emitter& out,
        const glm::mat4& mat
    );

    class Encoder
    {
    public:
        Encoder() = default;
        ~Encoder() = default;
    };
}
