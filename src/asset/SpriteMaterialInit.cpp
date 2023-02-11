#include "SpriteMaterialInit.h"

#include "Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"
#include "SpriteMesh.h"

#include "asset/MaterialIndex.h"

namespace {
}

void SpriteMaterialInit::prepare(
    SpriteMesh& mesh,
    MaterialVBO& materialVBO)
{
    prepareVertices(materialVBO);
}

void SpriteMaterialInit::prepareVertices(
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
