#pragma once

#include <ki/uuid.h>

#include "InstancedController.h"


class Registry;

class AsteroidBeltController final : public InstancedController
{
    struct Asteroid {
        int m_asteroidID;
        glm::vec3 m_position;
        float m_rotationAngle;
        float m_scale;
        float m_speed;
        int m_entityIndex;
    };

public:
    AsteroidBeltController(int asteroidCount);

    virtual void prepare(
        const Assets& assets,
        Registry* registry,
        Node& node) override;

    virtual bool update(
        const RenderContext& ctx,
        Node& node,
        Node* parent) override;

private:
    void updateAsteroids(
        const RenderContext& ctx,
        Node& node,
        Node* parent,
        bool rotate);

    void createAsteroids(
        const Assets& assets,
        Registry* registry,
        Node& node);

    void initAsteroids(
        const Assets& assets,
        Registry* registry,
        Node& node,
        std::vector<Asteroid>& asteroids);

    void rotateAsteroids(
        const RenderContext& ctx,
        Node& node,
        std::vector<Asteroid>& asteroids);

    void calculateVolume(
        Node& node,
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

