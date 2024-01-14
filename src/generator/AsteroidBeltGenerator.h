#pragma once

#include <ki/uuid.h>

#include "NodeGenerator.h"

struct PrepareContext;

class AsteroidBeltGenerator final : public NodeGenerator
{
public:
    AsteroidBeltGenerator(int asteroidCount);

    virtual void prepare(
        const PrepareContext& ctx,
        Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        Node& container) override;

private:
    void updateAsteroids(
        const UpdateContext& ctx,
        Node& container,
        bool rotate);

    void createAsteroids(
        const PrepareContext& ctx,
        Node& container);

    void initAsteroids(
        const PrepareContext& ctx,
        Node& container);

    void rotateAsteroids(
        const UpdateContext& ctx,
        Node& container);

private:
    const int m_asteroidCount;
    const float m_radius;
    const float m_offset;
    const int m_updateStep;

    int m_updateIndex = 0;
};

