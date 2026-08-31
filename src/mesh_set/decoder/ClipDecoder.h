#pragma once

#include "Decoder.h"

namespace animation
{
    struct Clip;
}

namespace mesh_set::decoder
{
    class ClipDecoder : public Decoder
    {
    public:
        ClipDecoder();
        ~ClipDecoder();

        void decode(
            const YAML::Node& node,
            animation::Clip& clip);
    };
}
