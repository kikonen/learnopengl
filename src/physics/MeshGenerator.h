#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>

#include <ode/ode.h>

#include "mesh/MeshInstance.h"

#include "size.h"

namespace mesh {
    class Mesh;
    class MeshType;
}

namespace physics {
    class PhysicsSystem;

    class MeshGenerator {
    public:
        MeshGenerator(const PhysicsSystem& physicsSystem);

        void clear();
        std::shared_ptr<std::vector<mesh::MeshInstance>> generateMeshes(bool onlyNavMesh);

    private:
        mesh::MeshInstance generateMesh(
            physics::GeomType geomType,
            dGeomID geomId);

        std::shared_ptr<mesh::Mesh> findMesh(const std::string& key);

        std::shared_ptr<mesh::Mesh> saveMesh(
            const std::string& key,
            std::shared_ptr<mesh::Mesh> mesh);

    private:
        const PhysicsSystem& m_physicsSystem;

        std::map<std::string, std::shared_ptr<mesh::Mesh>> m_cache;
    };
}
