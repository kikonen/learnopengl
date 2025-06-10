#pragma once

#include <vector>

#include "BaseLoader.h"
#include "GeneratorData.h"
#include "MaterialData.h"

class NodeGenerator;

class NodeType;

namespace loader {
    class Loaders;

    class GeneratorLoader : public BaseLoader
    {
    public:
        GeneratorLoader(
            std::shared_ptr<Context> ctx);

        void loadGenerator(
            const loader::DocNode& node,
            GeneratorData& data,
            Loaders& loaders) const;

        void loadTerrain(
            const loader::DocNode& node,
            TerrainData& data) const;

        std::unique_ptr<NodeGenerator> createGenerator(
            const GeneratorData& data,
            Loaders& loaders);
    };
}
