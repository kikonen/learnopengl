#include "ModelMeshEncoder.h"

#include <algorithm>
#include <functional>

#include "animation/VertexJoint.h"

#include "mesh/ModelMesh.h"

#include "MaterialEncoder.h"

namespace mesh_set::encoder
{
    void encodeVertices(
        YAML::Emitter& out,
        const std::string& key,
        const std::vector<mesh::Vertex>& vertices,
        const std::function<void(YAML::Emitter&, const mesh::Vertex&)>& fn
    )
    {
        out << YAML::Key << key;
        out << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (const auto& v : vertices) {
            fn(out, v);
        }
        out << YAML::EndSeq;
    }

    void encodeVertexJoints(
        YAML::Emitter& out,
        const std::string& key,
        const std::vector<animation::VertexJoint>& joints,
        const std::function<void(YAML::Emitter&, const animation::VertexJoint&)>& fn
    )
    {
        out << YAML::Key << key;
        out << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (const auto& v : joints) {
            fn(out, v);
        }
        out << YAML::EndSeq;
    }

    ModelMeshEncoder::ModelMeshEncoder() = default;
    ModelMeshEncoder::~ModelMeshEncoder() = default;

    void ModelMeshEncoder::encode(
        YAML::Emitter& out,
        const util::Ref<mesh::ModelMesh>& mesh)
    {
        out << YAML::BeginMap;
        {
            out << YAML::Key << "name";
            out << YAML::Value << mesh->m_name;

            out << YAML::Key << "alias";
            out << YAML::Value << mesh->m_alias;

            out << YAML::Key << "vertex_count";
            out << YAML::Value << mesh->m_vertices.size();
        }
        {
            encodeVertices(
                out,
                "positions",
                mesh->m_vertices,
                [](YAML::Emitter& out, const auto& v) {
                    encodeVec3(out, v.pos);
                });

            encodeVertices(
                out,
                "tex_coords",
                mesh->m_vertices,
                [](YAML::Emitter& out, const auto& v) {
                encodeVec2(out, v.texCoord);
            });

            encodeVertices(
                out,
                "normals",
                mesh->m_vertices,
                [](YAML::Emitter& out, const auto& v) {
                encodeVec3(out, v.normal);
            });

            encodeVertices(
                out,
                "tangents",
                mesh->m_vertices,
                [](YAML::Emitter& out, const auto& v) {
                encodeVec3(out, v.tangent);
            });

            encodeVertices(
                out,
                "bitangents",
                mesh->m_vertices,
                [](YAML::Emitter& out, const auto& v) {
                encodeVec3(out, v.bitangent);
            });

            encodeVertexJoints(
                out,
                "joint_ids",
                mesh->m_vertexJoints,
                [](YAML::Emitter& out, const auto& v) {
                encodeVec4(out, v.m_jointIds);
            });
            encodeVertexJoints(
                out,
                "joint_weights",
                mesh->m_vertexJoints,
                [](YAML::Emitter& out, const auto& v) {
                encodeVec4(out, v.m_weights);
            });
        }
        {
            out << YAML::Key << "indeces";
            out << YAML::Value << YAML::Flow << YAML::BeginSeq;
            for (const auto& v : mesh->m_indeces) {
                out << v;
            }
            out << YAML::EndSeq;
        }
        {
            out << YAML::Key << "material";
            out << YAML::Value;

            MaterialEncoder encoder;
            encoder.encode(out, mesh->getMaterial());
        }

        out << YAML::EndMap;
    }
}
