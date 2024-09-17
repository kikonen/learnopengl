#include "RigJointTreeGenerator.h"

#include <map>

#include "material/Material.h"

#include "shader/Shader.h"

#include "animation/RigContainer.h"
#include "animation/BoneInfo.h"

#include "mesh/PrimitiveMesh.h"

namespace mesh {
    std::unique_ptr<mesh::Mesh> RigJointTreeGenerator::generateTree(std::shared_ptr<animation::RigContainer> rigPtr) const
    {
        auto& rig = *rigPtr;

        auto mesh = std::make_unique<mesh::PrimitiveMesh>("joint_tree");

        mesh->m_rig = rigPtr;
        mesh->m_type = mesh::PrimitiveType::lines;

        auto& vertices = mesh->m_vertices;
        auto& vertexBones = mesh->m_vertexBones;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(rig.m_joints.size());
        vertexBones.reserve(rig.m_joints.size());
        indeces.reserve(rig.m_joints.size());

        std::map<int16_t, int16_t> jointToVertex;

        auto findVertexIndex = [&jointToVertex](uint16_t jointIndex) {
            const auto& it = jointToVertex.find(jointIndex);
            return it != jointToVertex.end() ? it->second : -1;
        };

        // generate initial vertices
        for (auto& rigJoint : rig.m_joints) {
            if (rigJoint.m_boneIndex < 0) continue;

            jointToVertex.insert({ rigJoint.m_index, static_cast<int16_t>(vertices.size()) });
            {
                auto& vertex = vertices.emplace_back();
                const auto* bone = rig.m_boneContainer.getInfo(rigJoint.m_boneIndex);
                vertex.pos = glm::inverse(bone->m_offsetMatrix) * glm::vec4{ 0, 0, 0, 1 };
            }
            {
                auto& vertexBone = vertexBones.emplace_back();
                vertexBone.addBone(rigJoint.m_boneIndex, 1.f);
            }
        }

        // generate lines: from child to parent
        for (auto& rigJoint : rig.m_joints) {
            if (rigJoint.m_parentIndex < 0) continue;
            if (rigJoint.m_boneIndex < 0) continue;

            int16_t vertexIndex = findVertexIndex(rigJoint.m_index);
            assert(vertexIndex >= 0);

            int parentIndex = rigJoint.m_parentIndex;
            while (parentIndex >= 0) {
                auto& parentJoint = rig.m_joints[parentIndex];

                // NOTE KI MUST skip non bone parents
                if (parentJoint.m_boneIndex >= 0) {
                    int16_t parentVertexIndex = findVertexIndex(parentJoint.m_index);
                    assert(parentVertexIndex >= 0);

                    indeces.push_back(parentVertexIndex);
                    indeces.push_back(vertexIndex);
                    break;
                }

                parentIndex = parentJoint.m_parentIndex;
            }
        }

        auto material = Material::createMaterial(BasicMaterial::blue);
        material.inmutable = true;
        material.m_programDefinitions.insert({ DEF_USE_BONES, "1" });
        material.m_programNames.insert({ MaterialProgramType::shader, "g_tex" });
        material.m_programNames.insert({ MaterialProgramType::shadow, "simple_depth" });
        mesh->setMaterial(&material);

        return mesh;
    }

    std::unique_ptr<mesh::Mesh> RigJointTreeGenerator::generatePoints(std::shared_ptr<animation::RigContainer> rigPtr) const
    {
        auto& rig = *rigPtr;

        auto mesh = std::make_unique<mesh::PrimitiveMesh>("joint_points");

        mesh->m_rig = rigPtr;
        mesh->m_type = mesh::PrimitiveType::points;

        auto& vertices = mesh->m_vertices;
        auto& vertexBones = mesh->m_vertexBones;
        auto& indeces = mesh->m_indeces;

        vertices.reserve(rig.m_joints.size());
        vertexBones.reserve(rig.m_joints.size());
        indeces.reserve(rig.m_joints.size());

        std::map<int16_t, int16_t> jointToVertex;

        auto findVertexIndex = [&jointToVertex](uint16_t jointIndex) {
            const auto& it = jointToVertex.find(jointIndex);
            return it != jointToVertex.end() ? it->second : -1;
            };

        // generate initial vertices
        for (auto& rigJoint : rig.m_joints) {
            if (rigJoint.m_boneIndex < 0) continue;

            jointToVertex.insert({ rigJoint.m_index, static_cast<int16_t>(vertices.size()) });
            {
                auto& vertex = vertices.emplace_back();
                const auto* bone = rig.m_boneContainer.getInfo(rigJoint.m_boneIndex);
                vertex.pos = glm::inverse(bone->m_offsetMatrix) * glm::vec4{ 0, 0, 0, 1 };
            }
            {
                auto& vertexBone = vertexBones.emplace_back();
                vertexBone.addBone(rigJoint.m_boneIndex, 1.f);
            }
        }

        // generate points: one for each vertex
        for (auto& rigJoint : rig.m_joints) {
            if (rigJoint.m_parentIndex < 0) continue;
            if (rigJoint.m_boneIndex < 0) continue;

            int16_t vertexIndex = findVertexIndex(rigJoint.m_index);
            assert(vertexIndex >= 0);

            indeces.push_back(vertexIndex);
        }

        {
            auto material = Material::createMaterial(BasicMaterial::green);

            material.inmutable = true;
            material.m_programDefinitions.insert({ DEF_USE_BONES, "1" });
            material.m_programNames.insert({ MaterialProgramType::shader, "g_tex" });
            material.m_programNames.insert({ MaterialProgramType::shadow, "simple_depth" });
            material.layersDepth = 6.f;

            mesh->setMaterial(&material);
        }

        return mesh;
    }
}
