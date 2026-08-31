#pragma once

#include "Decoder.h"

namespace animation
{
    struct Animation;
}

namespace mesh_set::decoder
{
    class AnimationDecoder : public Decoder
    {
    public:
        AnimationDecoder();
        ~AnimationDecoder();

        void decode(
            const YAML::Node& node,
            animation::Animation& animation);
    };
}
