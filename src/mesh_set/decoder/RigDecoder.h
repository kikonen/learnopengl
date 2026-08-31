#pragma once

#include "util/Ref.h"

#include "Decoder.h"

namespace animation
{
    struct Rig;
}

namespace mesh_set::decoder
{
    class RigDecoder : public Decoder
    {
    public:
        RigDecoder();
        ~RigDecoder();

        void decode(
            const YAML::Node& node,
            const util::Ref<animation::Rig>& rig);

    private:
        void decodeAnimations(
            const YAML::Node& nodes,
            const util::Ref<animation::Rig>& rig);
    };
}
