#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

struct Vertex;
class SpriteMesh;
class MaterialVBO;

class SpriteMaterialInit {
public:
    virtual void prepare(
        SpriteMesh& mesh,
        MaterialVBO& materialVBO);

private:
    void prepareVertices(
        MaterialVBO& materialVBO);
};
