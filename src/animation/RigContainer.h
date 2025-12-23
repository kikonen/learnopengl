#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "ki/size.h"

#include "material/Material.h"

#include "RigNode.h"
#include "RigSocket.h"
#include "MeshInfo.h"
#include "Clip.h"

#include "JointContainer.h"
#include "ClipContainer.h"

struct aiNode;
struct aiBone;

namespace mesh
{
    class ModelMesh;
}

namespace mesh_set
{
    class AssimpImporter;
    class AnimationImporter;
    class RigNodeTreeGenerator;
}

namespace animation
{
    class AnimationSystem;
    class Animator;

    struct RigContainer
    {
        friend class mesh_set::AssimpImporter;
        friend class mesh_set::AnimationImporter;
        friend class mesh_set::RigNodeTreeGenerator;
        friend class AnimationSystem;
        friend class Animator;

    public:
        RigContainer(const std::string& name);
        ~RigContainer();

        const std::string& getName() const noexcept
        {
            return m_name;
        }

        // @return true if rig is empty (thus no joints, and thus rig is not needed)
        bool empty() const noexcept
        {
            return false;
        }

        // finalizes container; no changes after this
        void prepare();
        void validate() const;

        void dump() const;

    private:
        const std::string m_name;
        bool m_prepared{ false };
    };
}
