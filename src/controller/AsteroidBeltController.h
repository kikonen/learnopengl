#pragma once

#include <stduuid/uuid.h>

#include "InstancedController.h"



class AsteroidBeltController final : public InstancedController
{
    struct Asteroid {
        int m_asteroidID;
        glm::vec3 m_position;
        float m_rotationAngle;
        float m_scale;
        float m_speed;
    };

public:
    AsteroidBeltController(int asteroidCount);

protected:
    void prepareInstanced(
        const Assets& assets,
        InstancedNode& node) override;

    bool updateInstanced(
        const RenderContext& ctx,
        InstancedNode& node,
        Node* parent) override;

private:
    void updateAsteroids(
        const RenderContext& ctx,
        InstancedNode& node,
        Node* parent);

    void initAsteroids(
        const Assets& assets,
        InstancedNode& node,
        std::vector<Asteroid>& asteroids);

    void rotateAsteroids(
        const RenderContext& ctx,
        InstancedNode& node,
        std::vector<Asteroid>& asteroids);

    void calculateVolume(
        InstancedNode& node,
        std::vector<Asteroid> asteroids);

private:
    const int m_asteroidCount;
    const int m_radius;
    const int m_offset;
    const int m_updateStep;

    int m_updateIndex = 0;

    std::vector<Asteroid> m_asteroids;
};

