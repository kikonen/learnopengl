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
        auto& entries = materialVBO.m_entries;
        entries.reserve(vertices.size());

        for (int i = 0; i < vertices.size(); i++) {
            MaterialEntry entry;

            const auto& vertex = vertices[i];
            auto* m = Material::findID(vertex.materialID, materialVBO.m_materials);

            if (materialVBO.m_useDefaultMaterial) {
                if (materialVBO.m_forceDefaultMaterial) {
                    m = &materialVBO.m_materials[0];
                }
            }

            // TODO KI should use noticeable value for missing
            // => would trigger undefined array access in render side
            entry.material = m ? m->m_registeredIndex : Material::DEFAULT_ID;

            assert(entry.material >= 0 && entry.material < MAX_MATERIAL_COUNT);

            entries.push_back(entry);
        }
    }
}
