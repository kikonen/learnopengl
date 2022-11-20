#include "AsteroidBeltController.h"

#include <algorithm>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "model/InstancedNode.h"

AsteroidBeltController::AsteroidBeltController(int asteroidCount)
    : m_asteroidCount(asteroidCount),
    m_radius(70.0),
    m_offset(20.5f),
    m_updateStep(3)
{
}

void AsteroidBeltController::prepareInstanced(
    const Assets& assets,
    InstancedNode& node)
{
    const int count = m_asteroidCount;

    for (size_t i = 0; i < count; i++)
    {
        m_asteroids.emplace_back();
    }

    initAsteroids(assets, node, m_asteroids);
    calculateVolume(node, m_asteroids);
}

bool AsteroidBeltController::updateInstanced(
    const RenderContext& ctx,
    InstancedNode& node,
    Node* parent)
{
    const bool changed = m_updateIndex% m_updateStep == 0 || node.getMatrixLevel() != m_nodeMatrixLevel;

    if (changed) {
        updateAsteroids(ctx, node, parent);
    }

    m_updateIndex++;
    m_nodeMatrixLevel = node.getMatrixLevel();

    return changed;
}

void AsteroidBeltController::updateAsteroids(
    const RenderContext& ctx,
    InstancedNode& node,
    Node* parent)
{
    node.clear();
    rotateAsteroids(ctx, node, m_asteroids);

    for (const auto& asteroid : m_asteroids)
    {
        glm::mat4 modelMat{ 1.f };
        {
            //modelMat = glm::translate(modelMat, asteroid.m_position + parentPos);
            modelMat = glm::translate(modelMat, asteroid.m_position);
            modelMat = glm::scale(modelMat, glm::vec3(asteroid.m_scale));
            modelMat = glm::rotate(modelMat, asteroid.m_rotationAngle, glm::vec3(0.4f, 0.6f, 0.8f));
        }

        const auto worldModelMat = parent->getWorldModelMatrix() * modelMat;
        // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
        const auto worldNormalMat = glm::inverseTranspose(glm::mat3(worldModelMat));

        node.add(worldModelMat, worldNormalMat, node.m_objectID);
    }
}

void AsteroidBeltController::initAsteroids(
    const Assets& assets,
    InstancedNode& node,
    std::vector<Asteroid>& asteroids)
{
    int count = asteroids.size();

    // initialize random seed
    srand(glfwGetTime());

    const glm::vec3 center{ 0.f };
    const float radius = m_radius;
    const float offset = m_offset;

    for (size_t i = 0; i < count; i++)
    {
        Asteroid& asteroid = asteroids[i];

        {
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)count * 360.0f;

            float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;

            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z

            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float z = cos(angle) * radius + displacement;

            asteroid.m_position = { x, y, z };
        }

        {
            // 2. scale: scale between 0.05 and 0.25f
            float scale = (rand() % 20) / 100.0f + 0.05f;
            asteroid.m_scale = scale;
        }

        {
            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            float rotAngle = (rand() % 360);
            asteroid.m_rotationAngle = rotAngle;
        }

        {
            // 3. make asteroids to rotate slowly
            float speed = (rand() % 200) / 20000.0f + 0.0005f;
            asteroid.m_speed = speed;
        }
    }
}

void AsteroidBeltController::rotateAsteroids(
    const RenderContext& ctx,
    InstancedNode& node,
    std::vector<Asteroid>& asteroids)
{
    const float elapsed = ctx.m_clock.elapsedSecs;
    const int count = asteroids.size();

    for (size_t i = 0; i < count; i++)
    {
        Asteroid& asteroid = asteroids[i];

        glm::mat4 modelMat{ 1.f };
        float angle = asteroid.m_speed * elapsed;

        auto mat = glm::toMat4(glm::quat(glm::vec3(0.f, angle, 0.f)));
        asteroid.m_position = mat * glm::vec4(asteroid.m_position, 0.f);
    }
}

void AsteroidBeltController::calculateVolume(
    InstancedNode& node,
    std::vector<Asteroid> asteroids)
{
    glm::vec3 minAABB = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 maxAABB = glm::vec3(std::numeric_limits<float>::min());

    for (auto&& asteroid : asteroids)
    {
        auto& pos = asteroid.m_position;

        minAABB.x = std::min(minAABB.x, pos.x);
        minAABB.y = std::min(minAABB.y, pos.y);
        minAABB.z = std::min(minAABB.z, pos.z);

        maxAABB.x = std::max(maxAABB.x, pos.x);
        maxAABB.y = std::max(maxAABB.y, pos.y);
        maxAABB.z = std::max(maxAABB.z, pos.z);
    }

    auto volume = std::make_unique<Sphere>(
        (maxAABB + minAABB) * 0.5f,
        glm::length(minAABB - maxAABB));
    node.setVolume(std::move(volume));

    AABB aabb{ minAABB, maxAABB, false };
    node.setAABB(aabb);
}
