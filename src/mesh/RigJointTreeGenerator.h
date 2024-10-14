#pragma once

#include <memory>

namespace animation {
    struct RigContainer;
}

namespace mesh {
    class VaoMesh;

    class RigJointTreeGenerator {
    public:
        RigJointTreeGenerator() = default;

        std::unique_ptr<mesh::VaoMesh> generateTree(
            std::shared_ptr<animation::RigContainer> rig) const;

        std::unique_ptr<mesh::VaoMesh> generatePoints(
            std::shared_ptr<animation::RigContainer> rig) const;

    private:
    };
}
