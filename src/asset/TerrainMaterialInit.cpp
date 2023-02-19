#include "TerrainMaterialInit.h"

#include "Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"
#include "TerrainMesh.h"

#include "asset/MaterialIndex.h"

namespace {
}

void TerrainMaterialInit::prepare(
    TerrainMesh& mesh,
    MaterialVBO& materialVBO)
{
    prepareVertices(materialVBO);
}

void TerrainMaterialInit::prepareVertices(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getMaterials()[0];

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    {
        auto& entries = materialVBO.m_indeces;

        auto& entry = entries.emplace_back();
        entry.m_materialIndex = material.m_registeredIndex;
    }
}
