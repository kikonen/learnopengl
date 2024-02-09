#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

namespace mesh {
    struct Vertex;
    class ModelMesh;
    class MaterialSet;

    class ModelMaterialInit {
    public:
        void prepare(
            ModelMesh& mesh,
            MaterialSet& materialSet);

    private:
        void prepareVertices(
            std::vector<Vertex>& vertices,
            MaterialSet& materialSet);
    };
}
