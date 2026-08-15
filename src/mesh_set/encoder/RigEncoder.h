#pragma once

#include "util/Ref.h"

#include "Encoder.h"

namespace animation
{
    struct Rig;
}

namespace mesh_set::encoder
{
    class RigEncoder : public Encoder
    {
    public:
        RigEncoder();
        ~RigEncoder();

        void encode(
            YAML::Emitter& out,
            const util::Ref<animation::Rig>& rig);
    };
}
