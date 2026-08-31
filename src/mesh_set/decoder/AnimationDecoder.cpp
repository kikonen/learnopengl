#include "AnimationDecoder.h"

#include "animation/Animation.h"

namespace mesh_set::decoder
{
    AnimationDecoder::AnimationDecoder() = default;
    AnimationDecoder::~AnimationDecoder() = default;

    void AnimationDecoder::decode(
        const YAML::Node& node,
        animation::Animation& animation)
    {
    }
}
