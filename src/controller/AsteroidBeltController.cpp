#include "AsteroidBeltController.h"

#include <fmt/format.h>

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

    Batch& modelBatch = node.modelBatch;
    Batch& selectedBatch = node.selectedBatch;

    modelBatch.reserve(count);
    selectedBatch.reserve(count);

    const glm::mat4 MODEL_0{ 0 };
    const glm::mat3 NORMAL_0{ 0 };

    for (size_t i = 0; i < count; i++)
    {
        m_asteroids.emplace_back();

        modelBatch.add(MODEL_0, NORMAL_0, 0);
        selectedBatch.add(MODEL_0, NORMAL_0, 0);
    }
}

bool AsteroidBeltController::updateInstanced(
    const RenderContext& ctx,
    InstancedNode& node,
    Node* parent)
{
    //const bool changed = m_updateIndex % m_updateStep == 0;
    const bool changed = m_updateIndex == 0;
    //return false;
    if (changed) {
        updateAsteroids(ctx, node, parent);
        node.markBuffersDirty();
    }
    m_updateIndex++;

    return changed;
}

void AsteroidBeltController::updateAsteroids(
    const RenderContext& ctx,
    InstancedNode& node,
    Node* parent)
{
    std::cout << fmt::format("update asteroids: {}\n", m_asteroids.size());

    Batch& modelBatch = node.modelBatch;
    Batch& selectedBatch = node.selectedBatch;

    glm::vec3 parentPos{ 0 };
    if (parent)
        parentPos = parent->getWorldPos();

    modelBatch.clear();
    selectedBatch.clear();

    calculateAsteroids(ctx.assets, node, m_asteroids);

    for (const auto& asteroid : m_asteroids)
    {
        glm::mat4 modelMat{ 1.f };
        {
            modelMat = glm::translate(modelMat, asteroid.m_position + parentPos);
            modelMat = glm::scale(modelMat, glm::vec3(asteroid.m_scale));
            modelMat = glm::rotate(modelMat, asteroid.m_rotationAngle, glm::vec3(0.4f, 0.6f, 0.8f));
        }

        glm::mat3 normalMat = glm::transpose(glm::inverse(glm::mat3(modelMat)));

        modelBatch.add(modelMat, normalMat, node.objectID);
    }

    modelBatch.staticDrawCount = m_asteroids.size();
    selectedBatch.staticDrawCount = 0;// m_asteroids.size();
}

void AsteroidBeltController::calculateAsteroids(
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
            float speed = (rand() % 20) / 1000.0f + 0.005f;
            asteroid.m_speed = speed;
        }
    }
}
