#pragma once

#include <memory>

#include "NodeGenerator.h"

#include "physics/Shape.h"

struct ShapeDefinition;

//
// Instance node entities based into grid
//
class GridGenerator final : public NodeGenerator {
public:
    GridGenerator();

    void prepareWT(
        const PrepareContext& ctx,
        model::Node& container) override;

    void updateWT(
        const UpdateContext& ctx,
        const model::Node& container) override;

private:
    void updateInstances(
        const UpdateContext& ctx,
        const model::Node& container);

    void prepareInstances(
        const PrepareContext& ctx,
        const model::Node& container);

    void prepareGrid(
        const model::Node& container,
        std::vector<mesh::Transform>& transforms) const;

    void prepareRandom(
        const model::Node& container,
        std::vector<mesh::Transform>& transforms) const;

    void updateBounds(
        const UpdateContext& ctx,
        const model::Node& container);

public:
    glm::uvec3 m_seed{ 0 };

    glm::vec3 m_boundsDir{ 0.f, -1.f, 0.f };
    uint32_t m_boundsMask{ UINT_MAX };

    // count for random
    // 0 == use grid size
    int m_count{ 0 };

    int m_xCount{ 1 };
    int m_yCount{ 1 };
    int m_zCount{ 1 };

    float m_xStep{ 0 };
    float m_yStep{ 0 };
    float m_zStep{ 0 };

    std::unique_ptr<ShapeDefinition> m_shapeTemplate;

private:
    bool m_boundsSetupDone{ false };
    bool m_dynamicBounds{ false };
    bool m_staticBounds{ false };

    std::vector<physics::Shape> m_shapes;
};
