#pragma once

#include "util/Ref.h"

#include "Encoder.h"

namespace mesh
{
    class ModelMesh;
}

namespace mesh_set::encoder
{
    class ModelMeshEncoder : public Encoder
    {
    public:
        ModelMeshEncoder();
        ~ModelMeshEncoder();

        void encode(
            YAML::Emitter& out,
            const util::Ref<mesh::ModelMesh>& mesh);
    };
}
