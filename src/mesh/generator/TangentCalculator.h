#pragma once

namespace mesh
{
    class VaoMesh;

    struct TangentCalculator {
        static void calculateTangents(mesh::VaoMesh& mesh);
    };
}
