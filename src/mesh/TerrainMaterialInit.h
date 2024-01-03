#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"


namespace mesh {
    struct Vertex;
    class TerrainMesh;
    class MaterialVBO;

    class TerrainMaterialInit {
    public:
        virtual void prepare(
            TerrainMesh& mesh,
            MaterialVBO& materialVBO);
    };
}
