#include "RigContainer.h"

#include <tuple>

#include <assimp/scene.h>

#include <fmt/format.h>

#include "util/glm_format.h"
#include "util/util.h"

#include "Animation.h"
#include "RigNode.h"
#include "Joint.h"
#include "VertexJoint.h"

namespace {
}

namespace animation {
    RigContainer::RigContainer(const std::string& name)
        : m_name{ name }
    {
    }

    RigContainer::~RigContainer() = default;


}
