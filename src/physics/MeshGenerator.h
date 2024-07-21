#pragma once

#include <vector>
#include <unordered_map>
#include <mutex>
#include <memory>

namespace mesh {
    class Mesh;
    class MeshType;
}

namespace physics {
    class PhysicsEngine;
    struct Object;

    class MeshGenerator {
    public:
        MeshGenerator(const PhysicsEngine& engine);

        std::vector<std::unique_ptr<mesh::Mesh>> generateMeshes() const;

    private:
        std::unique_ptr<mesh::Mesh> generateObject(const Object& obj) const;

    private:
        const PhysicsEngine& m_engine;
    };
}
