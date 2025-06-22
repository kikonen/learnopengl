#pragma once

#include <string>
#include <memory>
#include <type_traits>

#include <glm/glm.hpp>

#include "generator/GeneratorMode.h"

#include "loader/Repeat.h"
#include "loader/TerrainTiling.h"

#include "PhysicsDefinition.h"

class NodeGenerator;
class NodeType;
struct Material;

enum class GeneratorType : std::underlying_type_t<std::byte> {
    none,
    grid,
    terrain,
    asteroid_belt,
    text,
};

struct TerrainData {
    std::string map_height;
};

struct GeneratorDefinition {
    GeneratorType m_type{ GeneratorType::none };

    int m_count{ 0 };
    float m_radius{ 0.f };

    GeneratorMode m_mode{ GeneratorMode::none };

    glm::vec3 m_offset{ 0.f };
    float m_scale{ 1.f };

    glm::uvec3 m_seed{ 0 };

    loader::Repeat m_repeat;
    loader::TerrainTiling m_tiling;

    std::string m_heightMap;

    std::unique_ptr<Material> m_material;

    GeomDefinition m_geom;

    glm::vec3 m_boundsDir{ 0.f, -1.f, 0.f };
    uint32_t m_boundsMask{ UINT_MAX };

    static std::unique_ptr<NodeGenerator> createGenerator(
        const NodeType* type);
};
