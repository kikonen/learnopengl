#pragma once

#include "util/Ref.h"

#include "Decoder.h"

namespace mesh
{
    class MeshSet;
}

namespace mesh_set::decoder
{
    class MeshSetDecoder : public Decoder
    {
    public:
        MeshSetDecoder();
        ~MeshSetDecoder();

        void decode(
            const YAML::Node& node,
            const util::Ref<mesh::MeshSet>& meshSet);

    private:
        void decodeAnimationPaths(
            const YAML::Node& nodes,
            const util::Ref<mesh::MeshSet>& meshSet);
    };
}
