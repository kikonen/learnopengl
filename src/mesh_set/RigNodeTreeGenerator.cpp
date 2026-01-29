#include "RigNodeTreeGenerator.h"

#include <map>

#include "material/Material.h"

#include "shader/Shader.h"

#include "animation/Rig.h"
#include "animation/RigNode.h"
#include "animation/JointContainer.h"
#include "animation/Joint.h"

#include "mesh/PrimitiveMesh.h"

namespace mesh_set
{
    std::unique_ptr<mesh::VaoMesh> RigNodeTreeGenerator::generateTree(
        const std::shared_ptr<animation::Rig>& rigPtr) const
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(
            fmt::format("joint_tree_{}", rigPtr->m_skeletonRootNodeName));

        auto& rig = *rigPtr;
        const auto& jointContainer = rig.getJointContainer();

        mesh->m_rig = rigPtr;
        mesh->m_type = mesh::PrimitiveType::lines;

        auto& vertices = mesh->m_vertices;
        auto& vertexJoints = mesh->m_vertexJoints;
        auto& indeces = mesh->m_indeces;

        const size_t expectedCount = rig.m_nodes.size();

        vertices.reserve(expectedCount);
        vertexJoints.reserve(expectedCount);
        indeces.reserve(expectedCount * 2);

        std::map<int16_t, int16_t> nodeToVertex;

        auto findVertexIndex = [&nodeToVertex](int16_t nodeIndex) {
            const auto& it = nodeToVertex.find(nodeIndex);
            return it != nodeToVertex.end() ? it->second : -1;
        };

            // generate initial vertices
        for (auto& rigNode : rig.m_nodes) {
            const auto* joint = jointContainer.findByNodeIndex(rigNode.m_index);
            if (!joint) continue;

            nodeToVertex.insert({ rigNode.m_index, static_cast<int16_t>(vertices.size()) });

            {
                auto& vertex = vertices.emplace_back();
                vertex.pos = glm::inverse(joint->m_offsetMatrix) * glm::vec4{ 0, 0, 0, 1 };
            }
            {
                auto& vertexJoint = vertexJoints.emplace_back();
                vertexJoint.addJoint(joint->m_jointIndex, 1.f);
            }
        }

        // generate lines: from child to parent
        for (auto& rigNode : rig.m_nodes) {
            if (rigNode.m_parentIndex < 0) continue;

            const auto* joint = jointContainer.findByNodeIndex(rigNode.m_index);
            if (!joint) continue;

            int16_t vertexIndex = findVertexIndex(rigNode.m_index);
            assert(vertexIndex >= 0);

            int parentIndex = rigNode.m_parentIndex;
            while (parentIndex >= 0) {
                auto& parentNode = rig.m_nodes[parentIndex];

                // NOTE KI MUST skip non joint parents
                if (parentNode.m_hasJoint) {
                    int16_t parentVertexIndex = findVertexIndex(parentNode.m_index);
                    assert(parentVertexIndex >= 0);

                    indeces.push_back(parentVertexIndex);
                    indeces.push_back(vertexIndex);
                    break;
                }

                parentIndex = parentNode.m_parentIndex;
            }
        }

        auto material = Material::createMaterial(BasicMaterial::blue);
        material.inmutable = true;
        material.m_programDefinitions.insert({ DEF_USE_JOINTS, "1" });
        material.m_programNames.insert({ MaterialProgramType::shader, SHADER_G_TEX });
        material.m_programNames.insert({ MaterialProgramType::shadow, SHADER_SHADOW });
        mesh->setMaterial(&material);

        return mesh;
    }

    std::unique_ptr<mesh::VaoMesh> RigNodeTreeGenerator::generatePoints(
        const std::shared_ptr<animation::Rig>& rigPtr) const
    {
        auto mesh = std::make_unique<mesh::PrimitiveMesh>(
            fmt::format("joint_points_{}", rigPtr->m_skeletonRootNodeName));

        auto& rig = *rigPtr;
        const auto& jointContainer = rig.getJointContainer();

        mesh->m_rig = rigPtr;
        mesh->m_type = mesh::PrimitiveType::points;

        auto& vertices = mesh->m_vertices;
        auto& vertexJoints = mesh->m_vertexJoints;
        auto& indeces = mesh->m_indeces;

        const size_t expetedCount = rig.m_nodes.size();

        vertices.reserve(expetedCount);
        vertexJoints.reserve(expetedCount);
        indeces.reserve(expetedCount);

        std::map<int16_t, int16_t> nodeToVertex;

        auto findVertexIndex = [&nodeToVertex](uint16_t nodeIndex) {
            const auto& it = nodeToVertex.find(nodeIndex);
            return it != nodeToVertex.end() ? it->second : -1;
            };

        // generate initial vertices
        for (auto& rigNode : rig.m_nodes) {
            const auto* joint = jointContainer.findByNodeIndex(rigNode.m_index);
            if (!joint) continue;

            nodeToVertex.insert({ rigNode.m_index, static_cast<int16_t>(vertices.size()) });
            {
                auto& vertex = vertices.emplace_back();
                vertex.pos = glm::inverse(joint->m_offsetMatrix) * glm::vec4{ 0, 0, 0, 1 };
            }
            {
                auto& vertexJoint = vertexJoints.emplace_back();
                vertexJoint.addJoint(joint->m_jointIndex, 1.f);
            }
        }

        // generate points: one for each vertex
        for (auto& rigNode : rig.m_nodes) {
            if (rigNode.m_parentIndex < 0) continue;

            const auto* joint = jointContainer.findByNodeIndex(rigNode.m_index);
            if (!joint) continue;

            int16_t vertexIndex = findVertexIndex(rigNode.m_index);
            assert(vertexIndex >= 0);

            indeces.push_back(vertexIndex);
        }

        {
            auto material = Material::createMaterial(BasicMaterial::green);

            material.inmutable = true;
            material.m_programDefinitions.insert({ DEF_USE_JOINTS, "1" });
            material.m_programDefinitions.insert({ DEF_USE_GL_POINTS, "1" });
            material.m_programNames.insert({ MaterialProgramType::shader, SHADER_G_TEX });
            material.m_programNames.insert({ MaterialProgramType::shadow, SHADER_SHADOW });
            material.layersDepth = 6.f;

            mesh->setMaterial(&material);
        }

        return mesh;
    }
}
