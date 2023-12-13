#pragma once

#include "BaseLoader.h"
#include "GeneratorData.h"

class Node;
class NodeGenerator;

namespace loader {
    class GeneratorLoader : public BaseLoader
    {
    public:
        GeneratorLoader(
            Context ctx);

         void loadGenerator(
            const YAML::Node& node,
            GeneratorData& data) const;

       std::unique_ptr<NodeGenerator> createGenerator(
            const GeneratorData& data,
            Node* node);
    };
}
