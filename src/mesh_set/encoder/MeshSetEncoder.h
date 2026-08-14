#pragma once

#include "util/Ref.h"

#include "Encoder.h"

namespace mesh
{
    class MeshSet;
}

namespace mesh_set::encoder
{
    class MeshSetEncoder : public Encoder
    {
    public:
        MeshSetEncoder();
        ~MeshSetEncoder();

        void encode(
            YAML::Emitter& out,
            const util::Ref<mesh::MeshSet>& meshSet);
    };
}
