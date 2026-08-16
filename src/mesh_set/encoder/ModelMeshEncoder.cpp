#include "ModelMeshEncoder.h"

#include <algorithm>
#include <functional>

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/VertexJoint.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"

#include "mesh/ModelMesh.h"

#include "MaterialEncoder.h"

namespace mesh_set::encoder
{
    void encodeVertices(
        YAML::Emitter& out,
        const std::string& key,
        const std::vector<mesh::Vertex>& vertices,
        const std::function<void(std::vector<float>&, const mesh::Vertex&)>& fn
    )
    {
        std::vector<float> encoded;
        encoded.reserve(vertices.size() * 3);
        for (const auto& v : vertices) {
            fn(encoded, v);
        }

        out << YAML::Key << key;
        out << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (auto v : encoded) {
            out << v;
        }
        out << YAML::EndSeq;
    }

    void encodeIndeces(
        YAML::Emitter& out,
        const std::string& key,
        std::vector<mesh::Index32>& indeces,
        const std::function<void(std::vector<uint32_t>&, const mesh::Index32&)>& fn
    )
    {
        std::vector<uint32_t> encoded;
        encoded.reserve(indeces.size() * 3);
        for (const auto& v : indeces) {
            fn(encoded, v);
        }

        out << YAML::Key << key;
        out << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (auto v : encoded) {
            out << v;
        }
        out << YAML::EndSeq;
    }

    void encodeVertexJoints(
        YAML::Emitter& out,
        const std::string& key,
        const std::vector<animation::VertexJoint>& joints,
        const std::function<void(std::vector<float>&, const animation::VertexJoint&)>& fn
    )
    {
        std::vector<float> encoded;
        encoded.reserve(joints.size() * 4);
        for (const auto& v : joints) {
            fn(encoded, v);
        }

        out << YAML::Key << key;
        out << YAML::Value << YAML::Flow << YAML::BeginSeq;
        for (auto v : encoded) {
            out << v;
        }
        out << YAML::EndSeq;
    }

    void encodeJoints(
        YAML::Emitter& out,
        const std::string& key,
        const util::Ref<animation::JointContainer>& jointContainer
    )
    {
        if (jointContainer->empty()) return;

        out << YAML::Key << key;
        out << YAML::Value << YAML::BeginSeq;
        for (const auto& joint : jointContainer->m_joints) {
            out << YAML::BeginMap;
            {
                out << YAML::Key << "name";
                out << YAML::Value << joint.m_nodeName;

                out << YAML::Key << "offset";
                encodeMat4(out, joint.m_offsetMatrix);
            }
            out << YAML::EndMap;
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
                [](std::vector<float>& out, const auto& v) {
                    encodeVec3(out, v.pos);
                });

            encodeVertices(
                out,
                "tex_coords",
                mesh->m_vertices,
                [](std::vector<float>& out, const auto& v) {
                encodeVec2(out, v.texCoord);
            });

            encodeVertices(
                out,
                "normals",
                mesh->m_vertices,
                [](std::vector<float>& out, const auto& v) {
                encodeVec3(out, v.normal);
            });

            encodeVertices(
                out,
                "tangents",
                mesh->m_vertices,
                [](std::vector<float>& out, const auto& v) {
                encodeVec3(out, v.tangent);
            });

            encodeVertices(
                out,
                "bitangents",
                mesh->m_vertices,
                [](std::vector<float>& out, const auto& v) {
                encodeVec3(out, v.bitangent);
            });

            encodeVertexJoints(
                out,
                "joint_ids",
                mesh->m_vertexJoints,
                [](std::vector<float>& out, const auto& v) {
                encodeVec4(out, v.m_jointIds);
            });
            encodeVertexJoints(
                out,
                "joint_weights",
                mesh->m_vertexJoints,
                [](std::vector<float>& out, const auto& v) {
                encodeVec4(out, v.m_weights);
            });
        }
        {
            encodeIndeces(
                out,
                "indeces",
                mesh->m_indeces,
                [](std::vector<uint32_t>& out, const auto& v) {
                out.push_back(v);
            });
        }
        {
            out << YAML::Key << "material";
            out << YAML::Value;

            MaterialEncoder encoder;
            encoder.encode(out, mesh->getMaterial());
        }

        if (const auto& rig = mesh->getRig(); rig) {
            out << YAML::Key << "rig";
            out << YAML::Value << rig->getName();

            if (mesh->m_rigNodeIndex >= 0) {
                const auto* node = rig->getNode(mesh->m_rigNodeIndex);

                out << YAML::Key << "rig_node";
                out << YAML::Value << node->m_name;
            }

            out << YAML::Key << "joint_count";
            out << YAML::Value << mesh->getJointContainer()->m_joints.size();

            {
                encodeJoints(
                    out,
                    "joints",
                    mesh->getJointContainer()
                );
            }
        }

        out << YAML::EndMap;
    }
}
