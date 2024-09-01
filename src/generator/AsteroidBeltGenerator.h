#pragma once

#include "NodeGenerator.h"

struct PrepareContext;

class AsteroidBeltGenerator final : public NodeGenerator
{
public:
    AsteroidBeltGenerator(int asteroidCount);

    virtual void prepareWT(
        const PrepareContext& ctx,
        Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        const Node& container) override;

private:
    void updateAsteroids(
        const UpdateContext& ctx,
        const Node& container,
        bool rotate);

    void createAsteroids(
        const PrepareContext& ctx,
        const Node& container);

    void initAsteroids(
        const PrepareContext& ctx,
        const Node& container,
        std::vector<NodeState>& states);

    void rotateAsteroids(
        const UpdateContext& ctx,
        const Node& container);

private:
    const int m_asteroidCount;
    const float m_radius;
    const float m_offset;
    const int m_updateStep;

    int m_updateIndex = 0;
};

