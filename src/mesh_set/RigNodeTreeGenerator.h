#pragma once

#include <memory>

#include "util/Ref.h"

namespace animation
{
    struct Rig;
    struct JointContainer;
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

        util::Ref<mesh::VaoMesh> generateTree(
            const util::Ref<animation::Rig>& rig,
            const util::Ref<animation::JointContainer>& jointContainer) const;

        util::Ref<mesh::VaoMesh> generatePoints(
            const util::Ref<animation::Rig>& rig,
            const util::Ref<animation::JointContainer>& jointContainer) const;

    private:
    };
}
