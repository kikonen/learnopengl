#pragma once

#include "Encoder.h"

namespace animation
{
    struct Clip;
}

namespace mesh_set::encoder
{
    class ClipEncoder : public Encoder
    {
    public:
        ClipEncoder();
        ~ClipEncoder();

        void encode(
            YAML::Emitter& out,
            const animation::Clip& clip);
    };
}
