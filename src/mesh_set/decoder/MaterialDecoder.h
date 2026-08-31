#pragma once

#include "util/Ref.h"

#include "Decoder.h"

struct Material;

namespace mesh_set::decoder
{
    class MaterialDecoder : public Decoder
    {
    public:
        MaterialDecoder();
        ~MaterialDecoder();

        void decode(
            const YAML::Node& node,
            const util::Ref<Material>& material);
    };
}
