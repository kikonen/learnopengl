#pragma once

#include <vector>
#include <map>
#include <unordered_map>
#include <mutex>
#include <memory>

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

#include "util/Ref.h"

#include "mesh/MeshInstance.h"

#include "size.h"

namespace model
{
    class NodeType;
}

namespace mesh {
    class Mesh;
}

namespace physics {
    class PhysicsSystem;
    struct Object;

    class MeshGenerator {
    public:
        MeshGenerator(
            const PhysicsSystem& physicsSystem,
            int heightMapDivide);
        ~MeshGenerator();

        void clear();
        std::shared_ptr<std::vector<mesh::MeshInstance>> generateMeshes(bool onlyNavMesh);

    private:
        mesh::MeshInstance generateMesh(
            const physics::Object& obj,
            physics::object_id objectId);

        util::Ref<mesh::Mesh> findMesh(const std::string& key);

        util::Ref<mesh::Mesh> saveMesh(
            const std::string& key,
            const util::Ref<mesh::Mesh>& mesh);

    private:
        const PhysicsSystem& m_physicsSystem;

        std::map<std::string, util::Ref<mesh::Mesh>> m_cache;

        const int m_heightMapDivide;
    };
}
