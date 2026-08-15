#include "Encoder.h"

namespace mesh_set::encoder
{
    void encodeVec3(
        YAML::Emitter& out,
        const glm::vec3& vec
    )
    {
        out << YAML::Flow << YAML::BeginSeq;
        out << vec.x;
        out << vec.y;
        out << vec.z;
        out << YAML::EndSeq;
    }

    void encodeVec2(
        YAML::Emitter& out,
        const glm::vec2& vec
    )
    {
        out << YAML::Flow << YAML::BeginSeq;
        out << vec.x;
        out << vec.y;
        out << YAML::EndSeq;
    }

    void encodeVec4(
        YAML::Emitter& out,
        const glm::vec4& vec
    )
    {
        out << YAML::Flow << YAML::BeginSeq;
        out << vec.x;
        out << vec.y;
        out << vec.z;
        out << vec.w;
        out << YAML::EndSeq;
    }

    void encodeRGBA(
        YAML::Emitter& out,
        const glm::vec4& vec
    )
    {
        out << YAML::Flow << YAML::BeginSeq;
        out << vec.r;
        out << vec.g;
        out << vec.b;
        out << vec.a;
        out << YAML::EndSeq;
    }

    void encodeMat3(
        YAML::Emitter& out,
        const glm::mat3& mat
    )
    {
        out << YAML::Flow << YAML::BeginSeq;
        for (int i = 0; i < 3; i++) {
            const auto& vec = mat[i];
            out << YAML::Flow << YAML::BeginSeq;
            out << vec.x;
            out << vec.y;
            out << vec.z;
            out << YAML::EndSeq;
        }
        out << YAML::EndSeq;
    }

    void encodeMat4(
        YAML::Emitter& out,
        const glm::mat4& mat
    )
    {
        out << YAML::Flow << YAML::BeginSeq;
        for (int i = 0; i < 4; i++) {
            const auto& vec = mat[i];
            out << YAML::Flow << YAML::BeginSeq;
            out << vec.x;
            out << vec.y;
            out << vec.z;
            out << vec.w;
            out << YAML::EndSeq;
        }
        out << YAML::EndSeq;
    }

}
