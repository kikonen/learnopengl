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
        const RenderContext& ctx,
        Node& container,
        Node* containerParent) override;

private:
    void updateAsteroids(
        const RenderContext& ctx,
        Node& container,
        Node* containerParent,
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
        const RenderContext& ctx,
        Node& container);

private:
    const int m_asteroidCount;
    const float m_radius;
    const float m_offset;
    const int m_updateStep;

    int m_nodeMatrixLevel = 0;
    int m_updateIndex = 0;
};

