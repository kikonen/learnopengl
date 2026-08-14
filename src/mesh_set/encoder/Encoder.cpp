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
}
