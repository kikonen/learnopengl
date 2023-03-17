#pragma once

#include <ki/uuid.h>

#include "NodeGenerator.h"


class Registry;

class AsteroidBeltGenerator final : public NodeGenerator
{
public:
    AsteroidBeltGenerator(int asteroidCount);

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& container) override;

    virtual void update(
        const UpdateContext& ctx,
        Node& container) override;

private:
    void updateAsteroids(
        const UpdateContext& ctx,
        Node& container,
        bool rotate);

    void createAsteroids(
        const Assets& assets,
        Registry* registry,
        Node& container);

    void initAsteroids(
        const Assets& assets,
        Registry* registry,
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

