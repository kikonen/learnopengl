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
    class GeneratorLoader : public BaseLoader
    {
    public:
        GeneratorLoader(
            Context ctx);

        void loadGenerator(
            const YAML::Node& node,
            GeneratorData& data) const;

        void loadTerrain(
            const YAML::Node& node,
            TerrainData& data) const;

        std::unique_ptr<NodeGenerator> createGenerator(
            const GeneratorData& data,
            const std::vector<MaterialData>& materials,
            mesh::MeshType* type);
    };
}
