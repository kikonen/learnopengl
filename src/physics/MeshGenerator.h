#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>

#include "mesh/MeshInstance.h"

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

        void clear();
        std::shared_ptr<std::vector<mesh::MeshInstance>> generateMeshes();

    private:
        mesh::MeshInstance generateObject(const Object& obj);

        std::shared_ptr<mesh::Mesh> findMesh(const std::string& key);

        std::shared_ptr<mesh::Mesh> saveMesh(
            const std::string& key,
            std::shared_ptr<mesh::Mesh> mesh);

    private:
        const PhysicsEngine& m_engine;

        std::map<std::string, std::shared_ptr<mesh::Mesh>> m_cache;
    };
}
