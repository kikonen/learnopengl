#pragma once

#include <memory>

#include "NodeGenerator.h"

#include "physics/Geom.h"

struct GeomDefinition;

//
// Instance node entities based into grid
//
class GridGenerator final : public NodeGenerator {
public:
    GridGenerator();

    virtual void prepareWT(
        const PrepareContext& ctx,
        Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        const Node& container) override;

private:
    void updateInstances(
        const UpdateContext& ctx,
        const Node& container);

    void prepareInstances(
        const PrepareContext& ctx,
        const Node& container);

    void prepareGrid(
        const Node& container,
        std::vector<mesh::MeshTransform>& transforms) const;

    void prepareRandom(
        const Node& container,
        std::vector<mesh::MeshTransform>& transforms) const;

    void updateBounds(
        const UpdateContext& ctx,
        const Node& container);

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

    std::unique_ptr<GeomDefinition> m_geometryTemplate;

private:
    bool m_boundsSetupDone{ false };
    bool m_dynamicBounds{ false };
    bool m_staticBounds{ false };

    std::vector<physics::Geom> m_geometries;
};
