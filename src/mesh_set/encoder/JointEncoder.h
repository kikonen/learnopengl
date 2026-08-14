#pragma once

#include "Encoder.h"

namespace animation
{
    struct Joint;
}

namespace mesh_set::encoder
{
    class JointEncoder : public Encoder
    {
    public:
        JointEncoder();
        ~JointEncoder();

        void encode(
            YAML::Emitter& out,
            const animation::Joint& joint);
    };
}
