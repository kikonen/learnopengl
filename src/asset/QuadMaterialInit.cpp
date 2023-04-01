#include "QuadMaterialInit.h"

#include "Program.h"
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
    // NOTE KI *NO* indeces if single material
    //prepareVertices(materialVBO);
}

void QuadMaterialInit::prepareVertices(
    MaterialVBO& materialVBO)
{
    const auto& material = materialVBO.getFirst();

    // MaterialVBO
    // https://paroj.github.io/gltut/Basic%20Optimization.html
    // NOTE KI single DOES NOT work due to logic how intanced rendering
    // and glVertexArrayBindingDivisor work (MUST seemingly match instanced count)
    {
        const int count = 1;// VERTEX_COUNT;
        auto& entries = materialVBO.m_indeces;
        entries.reserve(count);

        for (int i = 0; i < count; i++) {
            // NOTE KI hardcoded single material
            entries.emplace_back(material.m_registeredIndex);
        }
    }
}
