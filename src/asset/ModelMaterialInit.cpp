#include "ModelMaterialInit.h"

#include "Shader.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"

namespace {
}

void ModelMaterialInit::prepare(
    ModelMesh& mesh,
    MaterialVBO& materialVBO)
{
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
        auto& entries = materialVBO.m_entries;
        entries.reserve(count);

        for (int i = 0; i < count; i++) {
            auto& entry = entries.emplace_back();

            const auto& vertex = vertices[i];
            auto* m = Material::findID(vertex.materialID, materialVBO.m_materials);

            if (materialVBO.m_useDefaultMaterial) {
                if (materialVBO.m_forceDefaultMaterial) {
                    m = &materialVBO.m_materials[0];
                }
            }

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            entry.materialIndex = m ? m->m_registeredIndex : Material::DEFAULT_ID;

            assert(entry.materialIndex >= 0 && entry.materialIndex < MAX_MATERIAL_COUNT);
        }

        materialVBO.m_singleMaterial = single;
    }
}
