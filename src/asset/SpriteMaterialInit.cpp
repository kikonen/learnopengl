#include "SpriteMaterialInit.h"

#include "Shader.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"
#include "SpriteMesh.h"

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
        auto& entries = materialVBO.m_entries;

        MaterialEntry entry;
        entry.material = material.m_registeredIndex;

        entries.push_back(entry);

        materialVBO.m_instanceDivisor = 0;
    }
}
