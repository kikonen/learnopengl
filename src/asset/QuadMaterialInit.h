#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

struct Vertex;
class QuadMesh;
class MaterialVBO;

class QuadMaterialInit {
public:
    virtual void prepare(
        QuadMesh& mesh,
        MaterialVBO& materialVBO);

private:
    void prepareVertices(
        MaterialVBO& materialVBO);
};
