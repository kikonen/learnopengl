#include "ModelMaterialInit.h"

#include "asset/Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"


namespace mesh {
    void ModelMaterialInit::prepare(
        ModelMesh& mesh,
        MaterialVBO& materialVBO)
    {
        KI_INFO(fmt::format("PREPARE_MATERIAL: mesh={}, materials={}", mesh.str(), materialVBO.getMaterialCount()));
        prepareVertices(mesh.m_vertices, materialVBO);
    }

    void ModelMaterialInit::prepareVertices(
        std::vector<Vertex>& vertices,
        MaterialVBO& materialVBO)
    {
        // https://paroj.github.io/gltut/Basic%20Optimization.html
        {
            const bool single = materialVBO.getMaterialCount() == 1;

            // NOTE KI *NO* indeces if single material
            if (single) {
                return;
            }

            const size_t vertexCount = vertices.size();
            auto& indeces = materialVBO.modifyIndeces();
            indeces.reserve(vertexCount);

            const auto& materials = materialVBO.getMaterials();

            for (size_t i = 0; i < vertexCount; i++) {
                const auto& vertex = vertices[i];
                auto* mat = Material::findID(vertex.materialID, materials);

                bool forcedDefault = false;
                if (materialVBO.isUseDefaultMaterial()) {
                    if (materialVBO.isForceDefaultMaterial()) {
                        mat = &materials[0];
                        forcedDefault = true;
                    }
                }

                //KI_INFO(fmt::format(
                //    "FIND_MATERIAL: matID={} => matIndex={}, forcedDef={}",
                //    vertex.materialID, mat->m_registeredIndex, forcedDefault));

                // TODO KI should use noticeable value for missing
                // => would trigger undefined array access in render side
                GLuint materialIndex = mat ? mat->m_registeredIndex : Material::DEFAULT_ID;
                indeces.emplace_back(materialIndex);
            }
        }
    }
}
