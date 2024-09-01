#pragma once

#include "NodeGenerator.h"

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
        std::vector<NodeState>& states) const;

    void prepareRandom(
        const Node& container,
        std::vector<NodeState>& states) const;

public:
    glm::uvec3 m_seed{ 0 };

    int m_xCount{ 1 };
    int m_yCount{ 1 };
    int m_zCount{ 1 };

    float m_xStep{ 0 };
    float m_yStep{ 0 };
    float m_zStep{ 0 };
};
