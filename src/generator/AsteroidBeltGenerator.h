#pragma once

#include "NodeGenerator.h"

#include "model/InstancePhysics.h"

struct PrepareContext;

class AsteroidBeltGenerator final : public NodeGenerator
{
public:
    AsteroidBeltGenerator(int asteroidCount);

    virtual void prepareWT(
        const PrepareContext& ctx,
        model::Node& container) override;

    virtual void updateWT(
        const UpdateContext& ctx,
        const model::Node& container) override;

private:
    void updateAsteroids(
        const UpdateContext& ctx,
        const model::Node& container,
        bool rotate);

    void createAsteroids(
        const PrepareContext& ctx,
        const model::Node& container);

    void initAsteroids(
        const PrepareContext& ctx,
        const model::Node& container,
        std::vector<mesh::Transform>& transforms);

    void rotateAsteroids(
        const UpdateContext& ctx,
        const model::Node& container);

private:
    const int m_asteroidCount;
    const float m_radius;
    const float m_modifier;
    const int m_updateStep;

    int m_updateIndex = 0;
    int m_strideIndex = 0;

    std::vector<InstancePhysics> m_physics;
};

