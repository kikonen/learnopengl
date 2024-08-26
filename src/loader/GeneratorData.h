#pragma once

#include <glm/glm.hpp>

#include "BaseData.h"

#include "generator/GeneratorMode.h"

#include "TerrainData.h"
#include "MaterialData.h"

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
        float radius{ 0.f };

        GeneratorMode mode{ GeneratorMode::none };

        glm::uvec3 seed{ 0 };

        Repeat repeat;
        Tiling tiling;

        TerrainData terrainData;

        MaterialData materialData;
    };
}
