#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

struct Vertex;
class ModelMesh;
class MaterialVBO;

class ModelMaterialInit {
public:
    void prepare(
        ModelMesh& mesh,
        MaterialVBO& materialVBO);

private:
    void prepareVertices(
        std::vector<Vertex>& vertices,
        MaterialVBO& materialVBO);
};
