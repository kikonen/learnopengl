#pragma once

#include <ki/uuid.h>

#include "NodeGenerator.h"


class Registry;

class AsteroidBeltGenerator final : public NodeGenerator
{
    struct Asteroid : NodeInstance {
        float m_angularVelocity;
    };

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
        Node& container,
        std::vector<Asteroid>& asteroids);

    void rotateAsteroids(
        const RenderContext& ctx,
        Node& container,
        std::vector<Asteroid>& asteroids);

    void calculateVolume(
        Node& container,
        std::vector<Asteroid> asteroids);

private:
    const int m_asteroidCount;
    const float m_radius;
    const float m_offset;
    const int m_updateStep;

    int m_nodeMatrixLevel = 0;
    int m_updateIndex = 0;

    int m_firstEntityIndex;

    std::vector<Asteroid> m_asteroids;
};

