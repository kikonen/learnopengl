#pragma once

namespace mesh
{
    class Mesh;

    struct TangentCalculator {
        static void calculateTangents(mesh::Mesh& mesh);
    };
}
