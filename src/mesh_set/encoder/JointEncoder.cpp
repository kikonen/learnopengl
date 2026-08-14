#include "JointEncoder.h"

#include "animation/Joint.h"

namespace mesh_set::encoder
{
    JointEncoder::JointEncoder() = default;
    JointEncoder::~JointEncoder() = default;

    void JointEncoder::encode(
        YAML::Emitter& out,
        const animation::Joint& Joint)
    {
    }
}
