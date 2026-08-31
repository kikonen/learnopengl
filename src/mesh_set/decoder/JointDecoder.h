#pragma once

#include "Decoder.h"

namespace animation
{
    struct Joint;
}

namespace mesh_set::decoder
{
    class JointDecoder : public Decoder
    {
    public:
        JointDecoder();
        ~JointDecoder();

        void decode(
            const YAML::Node& node,
            animation::Joint& joint);
    };
}
