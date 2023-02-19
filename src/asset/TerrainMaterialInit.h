#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

struct Vertex;
class TerrainMesh;
class MaterialVBO;

class TerrainMaterialInit {
public:
    virtual void prepare(
        TerrainMesh& mesh,
        MaterialVBO& materialVBO);

private:
    void prepareVertices(
        MaterialVBO& materialVBO);
};
