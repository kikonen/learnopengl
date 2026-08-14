#pragma once

#include "Encoder.h"

namespace animation
{
    struct Animation;
}

namespace mesh_set::encoder
{
    class AnimationEncoder : public Encoder
    {
    public:
        AnimationEncoder();
        ~AnimationEncoder();

        void encode(
            YAML::Emitter& out,
            const animation::Animation& animation);
    };
}
