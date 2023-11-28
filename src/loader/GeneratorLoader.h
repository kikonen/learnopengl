#pragma once

#include "BaseLoader.h"

class Node;
class NodeGenerator;

namespace loader {
    enum class GeneratorType {
        none,
        grid,
        terrain,
        asteroid_belt,
    };

    struct GeneratorData {
        bool enabled{ false };
        GeneratorType type{ GeneratorType::none };

        int count{ 0 };
        int mode{ 0 };
        float radius{ 0.f };

        Repeat repeat;
        Tiling tiling;
    };

    class GeneratorLoader : public BaseLoader
    {
    public:
        GeneratorLoader(
            Context ctx);

         void loadGenerator(
            const YAML::Node& node,
            GeneratorData& data);

       std::unique_ptr<NodeGenerator> createGenerator(
            const GeneratorData& data,
            Node* node);
    };
}
