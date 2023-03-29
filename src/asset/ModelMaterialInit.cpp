#include "ModelMaterialInit.h"

#include "asset/Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"

#include "asset/MaterialIndex.h"

namespace {
}

void ModelMaterialInit::prepare(
    ModelMesh& mesh,
    MaterialVBO& materialVBO)
{
    KI_INFO_OUT(fmt::format("PREPARE_MAT: {}", mesh.str()));
    prepareVertices(mesh.m_vertices, materialVBO);
}

void ModelMaterialInit::prepareVertices(
    std::vector<Vertex>& vertices,
    MaterialVBO& materialVBO)
{
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    {
        const bool single = materialVBO.m_materials.size() == 1;
        const size_t count = single ? 1 : vertices.size();
        auto& indeces = materialVBO.m_indeces;
        indeces.reserve(count);

        if (!single)
            int x = 0;

        for (int i = 0; i < count; i++) {
            auto& entry = indeces.emplace_back();

            const auto& vertex = vertices[i];
            auto* mat = Material::findID(vertex.materialID, materialVBO.m_materials);

            bool forcedDefault = false;
            if (materialVBO.m_useDefaultMaterial) {
                if (materialVBO.m_forceDefaultMaterial) {
                    mat = &materialVBO.m_materials[0];
                    forcedDefault = true;
                }
            }

            KI_INFO(fmt::format(
                "FIND: matID={} => matIndex={}, forcedDef={}",
                vertex.materialID, mat->m_registeredIndex, forcedDefault));

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            entry.m_materialIndex = mat ? mat->m_registeredIndex : Material::DEFAULT_ID;
        }

        materialVBO.m_singleMaterial = single;
    }
}
