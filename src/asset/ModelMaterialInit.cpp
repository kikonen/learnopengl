#include "ModelMaterialInit.h"

#include "asset/Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"


void ModelMaterialInit::prepare(
    ModelMesh& mesh,
    MaterialVBO& materialVBO)
{
    KI_INFO(fmt::format("PREPARE_MATERIAL: mesh={}, materials={}", mesh.str(), materialVBO.m_materials.size()));
    prepareVertices(mesh.m_vertices, materialVBO);
}

void ModelMaterialInit::prepareVertices(
    std::vector<Vertex>& vertices,
    MaterialVBO& materialVBO)
{
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    {
        const bool single = materialVBO.m_materials.size() == 1;

        // NOTE KI *NO* indeces if single material
        if (single) {
            return;
        }

        const size_t count = vertices.size();
        auto& indeces = materialVBO.m_indeces;
        indeces.reserve(count);

        for (int i = 0; i < count; i++) {
            const auto& vertex = vertices[i];
            auto* mat = Material::findID(vertex.materialID, materialVBO.m_materials);

            bool forcedDefault = false;
            if (materialVBO.m_useDefaultMaterial) {
                if (materialVBO.m_forceDefaultMaterial) {
                    mat = &materialVBO.m_materials[0];
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
