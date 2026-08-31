#pragma once

#include "util/Ref.h"

#include "Decoder.h"

namespace mesh
{
    class ModelMesh;
}

namespace mesh_set::decoder
{
    class ModelMeshDecoder : public Decoder
    {
    public:
        ModelMeshDecoder();
        ~ModelMeshDecoder();

        void decode(
            const YAML::Node& node,
            const util::Ref<mesh::ModelMesh>& mesh);
    };
}
