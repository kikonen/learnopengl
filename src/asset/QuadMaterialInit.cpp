#include "QuadMaterialInit.h"

#include "Shader.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"
#include "QuadMesh.h"

namespace {
    constexpr int VERTEX_COUNT = 4;
}

void QuadMaterialInit::prepare(
    QuadMesh& mesh,
    MaterialVBO& materialVBO)
{
    prepareVertices(materialVBO);
}

void QuadMaterialInit::prepareVertices(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getMaterials()[0];

    // MaterialVBO
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    // NOTE KI single DOES NOT work due to logic how intanced rendering
    // and glVertexArrayBindingDivisor work (MUST seemingly match instanced count)
    {
        const int count = 1;// VERTEX_COUNT;
        auto& entries = materialVBO.m_entries;
        entries.reserve(count);

        for (int i = 0; i < count; i++) {
            MaterialEntry entry;

            // NOTE KI hardcoded single material
            entry.materialIndex = material.m_registeredIndex;

            assert(entry.materialIndex >= 0 && entry.materialIndex < MAX_MATERIAL_COUNT);

            entries.push_back(entry);
        }
    }
}
