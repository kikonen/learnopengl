#pragma once

#include <vector>

#include "kigl/GLBuffer.h"
#include "kigl/GLVertexArray.h"

namespace mesh {
    struct Vertex;
    class QuadMesh;
    class MaterialVBO;

    class QuadMaterialInit {
    public:
        virtual void prepare(
            QuadMesh& mesh,
            MaterialVBO& materialVBO);
    };
}
