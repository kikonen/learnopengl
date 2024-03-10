#pragma once

#include "BaseData.h"

#include "TerrainData.h"

class Node;
class NodeGenerator;

namespace loader {
    enum class GeneratorType : std::underlying_type_t<std::byte> {
        none,
        grid,
        terrain,
        asteroid_belt,
        text,
    };

    struct GeneratorData {
        bool enabled{ false };
        GeneratorType type{ GeneratorType::none };

        int count{ 0 };
        int mode{ 0 };
        float radius{ 0.f };

        Repeat repeat;
        Tiling tiling;

        TerrainData terrainData;

        std::string materialName;
    };
}
