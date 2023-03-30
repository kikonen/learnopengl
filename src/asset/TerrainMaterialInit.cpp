#include "TerrainMaterialInit.h"

#include "Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"
#include "TerrainMesh.h"


void TerrainMaterialInit::prepare(
    TerrainMesh& mesh,
    MaterialVBO& materialVBO)
{
    prepareVertices(materialVBO);
}

void TerrainMaterialInit::prepareVertices(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getFirst();

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    {
        auto& entries = materialVBO.m_indeces;
        entries.emplace_back(material.m_registeredIndex);
    }
}
