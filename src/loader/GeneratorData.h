#pragma once

#include <type_traits>

#include <glm/glm.hpp>

#include "BaseData.h"

#include "generator/GeneratorMode.h"

#include "component/GeneratorDefinition.h"

#include "PhysicsData.h"
#include "TerrainData.h"
#include "MaterialData.h"

namespace loader {
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

        loader::GeomData geom;

        glm::vec3 boundsDir{ 0.f, -1.f, 0.f };
        uint32_t boundsMask{ UINT_MAX };
    };
}
