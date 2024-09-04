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

        glm::vec3 offset{ 0.f };
        float scale{ 1.f };

        glm::uvec3 seed{ 0 };

        Repeat repeat;
        Tiling tiling;

        TerrainData terrainData;

        MaterialData materialData;

        glm::vec3 boundsDir{ 0.f, -1.f, 0.f };
        uint32_t boundsMask{ UINT_MAX };
    };
}
