#include "AnimationEncoder.h"

#include "animation/Animation.h"

namespace mesh_set::encoder
{
    AnimationEncoder::AnimationEncoder() = default;
    AnimationEncoder::~AnimationEncoder() = default;

    void AnimationEncoder::encode(
        YAML::Emitter& out,
        const animation::Animation& animation)
    {
    }
}
