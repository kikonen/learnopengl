#include "JointDecoder.h"

#include "animation/Joint.h"

namespace mesh_set::decoder
{
    JointDecoder::JointDecoder() = default;
    JointDecoder::~JointDecoder() = default;

    void JointDecoder::decode(
        const YAML::Node& node,
        animation::Joint& joint)
    {
    }
}
