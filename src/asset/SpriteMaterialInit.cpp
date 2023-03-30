#include "SpriteMaterialInit.h"

#include "Program.h"
#include "ModelMesh.h"
#include "MaterialVBO.h"
#include "SpriteMesh.h"


void SpriteMaterialInit::prepare(
    SpriteMesh& mesh,
    MaterialVBO& materialVBO)
{
    prepareVertices(materialVBO);
}

void SpriteMaterialInit::prepareVertices(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getFirst();

    // https://paroj.github.io/gltut/Basic%20Optimization.html
    {
        auto& entries = materialVBO.m_indeces;
        entries.emplace_back(material.m_registeredIndex);
    }
}
