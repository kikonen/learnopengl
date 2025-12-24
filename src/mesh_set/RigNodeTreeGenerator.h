#pragma once

#include <memory>

namespace animation
{
    struct Rig;
}

namespace mesh
{
    class VaoMesh;
}

namespace mesh_set
{
    class RigNodeTreeGenerator
    {
    public:
        RigNodeTreeGenerator() = default;

        std::unique_ptr<mesh::VaoMesh> generateTree(
            const std::shared_ptr<animation::Rig>& rig) const;

        std::unique_ptr<mesh::VaoMesh> generatePoints(
            const std::shared_ptr<animation::Rig>& rig) const;

    private:
    };
}
