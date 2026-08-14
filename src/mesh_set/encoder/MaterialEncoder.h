#pragma once

#include "util/Ref.h"

#include "Encoder.h"

struct Material;

namespace mesh_set::encoder
{
    class MaterialEncoder : public Encoder
    {
    public:
        MaterialEncoder();
        ~MaterialEncoder();

        void encode(
            YAML::Emitter& out,
            const util::Ref<Material>& material);
    };
}
