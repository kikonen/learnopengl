#include "ModelMeshDecoder.h"

#include "mesh/ModelMesh.h"
#include "animation/JointContainer.h"
#include "animation/VertexJoint.h"

#include "MaterialDecoder.h"
#include "Decoder.h"

namespace mesh_set::decoder
{
    ModelMeshDecoder::ModelMeshDecoder() = default;
    ModelMeshDecoder::~ModelMeshDecoder() = default;

    void ModelMeshDecoder::decode(
        const YAML::Node& node,
        const util::Ref<mesh::ModelMesh>& mesh)
    {
        if (!node) return;

        auto& vertices = mesh->m_vertices;
        auto& indeces = mesh->m_indeces;

        for (const auto& pair : node) {
            const std::string& k = pair.first.as<std::string>();
            const auto& v = pair.second;

            if (k == "name") {
                mesh->m_name = v.as<std::string>();
                break;
            }
            if (k == "alias") {
                mesh->m_alias = v.as<std::string>();
                break;
            }
            if (k == "vertex_count") {
                const size_t vertexCount = v.as<size_t>();
                vertices.resize(vertexCount);
                break;
            }
            if (k == "positions") {
                const auto& values = decodeCompressedFloats(v);
                if (!values.empty()) {
                    for (size_t i = 0; i < vertices.size() && i * 3 + 2 < values.size(); ++i) {
                        vertices[i].pos = glm::vec3(values[i * 3], values[i * 3 + 1], values[i * 3 + 2]);
                    }
                }
                break;
            }
            if (k == "tex_coords") {
                const auto& values = decodeCompressedFloats(v);
                if (!values.empty()) {
                    for (size_t i = 0; i < vertices.size() && i * 2 + 1 < values.size(); ++i) {
                        vertices[i].texCoord = glm::vec2(values[i * 2], values[i * 2 + 1]);
                    }
                }
                break;
            }
            if (k == "normals") {
                const auto& values = decodeCompressedFloats(v);
                if (!values.empty()) {
                    for (size_t i = 0; i < vertices.size() && i * 3 + 2 < values.size(); ++i) {
                        vertices[i].normal = glm::vec3(values[i * 3], values[i * 3 + 1], values[i * 3 + 2]);
                    }
                }
                break;
            }
            if (k == "indeces") {
                const auto& values = decodeCompressedUint32(v);
                indeces.resize(values.size());
                for (size_t i = 0; i < indeces.size(); ++i) {
                    indeces[i] = static_cast<mesh::Index32>(values[i]);
                }
                break;
            }
            if (k == "material") {
                MaterialDecoder decoder;
                mesh->setMaterial(Material::createMaterial(BasicMaterial::basic));
                decoder.decode(v, mesh->getMaterial());
                break;
            }
        }
    }
}
