#pragma once

#include <vector>

#include "BaseLoader.h"
#include "GeneratorData.h"
#include "MaterialData.h"

class NodeGenerator;

namespace mesh {
    class MeshType;
}

namespace loader {
    class Loaders;

    class GeneratorLoader : public BaseLoader
    {
    public:
        GeneratorLoader(
            Context ctx);

        void loadGenerator(
            const loader::Node& node,
            GeneratorData& data,
            Loaders& loaders) const;

        void loadTerrain(
            const loader::Node& node,
            TerrainData& data) const;

        std::unique_ptr<NodeGenerator> createGenerator(
            const GeneratorData& data,
            mesh::MeshType* type);
    };
}
