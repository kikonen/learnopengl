#pragma once

#include <memory>

namespace animation {
    struct RigContainer;
}

namespace mesh {
    class Mesh;

    class RigJointTreeGenerator {
    public:
        RigJointTreeGenerator() = default;

        std::unique_ptr<mesh::Mesh> generate(std::shared_ptr<animation::RigContainer> rig) const;

    private:
    };
}
