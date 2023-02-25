#include "AsteroidBeltController.h"

#include <algorithm>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/Mesh.h"

#include "model/Node.h"

#include "scene/RenderContext.h"
#include "scene/Batch.h"

#include "registry/Registry.h"
#include "registry/EntityRegistry.h"
#include "registry/EntitySSBO.h"


AsteroidBeltController::AsteroidBeltController(int asteroidCount)
    : m_asteroidCount(asteroidCount),
    m_radius(70.0),
    m_offset(20.5f),
    m_updateStep(3)
{
}

void AsteroidBeltController::prepare(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    NodeController::prepare(assets, registry, node);

    createAsteroids(assets, registry, node);
}

bool AsteroidBeltController::update(
    const RenderContext& ctx,
    Node& node,
    Node* parent)
{
    const bool rotate = m_updateIndex% m_updateStep == 0 || node.getMatrixLevel() != m_nodeMatrixLevel;

    if (rotate) {
        updateAsteroids(ctx, node, parent, rotate);
    }

    m_updateIndex++;
    m_nodeMatrixLevel = node.getMatrixLevel();

    return rotate;
}

void AsteroidBeltController::updateAsteroids(
    const RenderContext& ctx,
    Node& node,
    Node* parent,
    bool rotate)
{
    auto* registry = ctx.m_registry;
    auto& entityRegistry = *registry->m_entityRegistry;

    if (rotate) {
        rotateAsteroids(ctx, node, m_asteroids);
    }

    for (const auto& asteroid : m_asteroids)
    {
        auto* entity = entityRegistry.updateEntity(asteroid.m_entityIndex, true);

        if (rotate) {
            glm::mat4 modelMatrix{ 1.f };
            {
                //modelMat = glm::translate(modelMat, asteroid.m_position + parentPos);
                modelMatrix = glm::translate(modelMatrix, asteroid.m_position);
                modelMatrix = glm::scale(modelMatrix, glm::vec3(asteroid.m_scale));
                modelMatrix = glm::rotate(modelMatrix, asteroid.m_rotationAngle, glm::vec3(0.4f, 0.6f, 0.8f));
                modelMatrix = parent->getModelMatrix() * modelMatrix;
            }

            entity->setModelMatrix(modelMatrix);
            // https://stackoverflow.com/questions/27600045/the-correct-way-to-calculate-normal-matrix
            entity->setNormalMatrix(glm::mat3(glm::inverseTranspose(modelMatrix)));
        }

        entity->u_materialIndex = node.getMaterialIndex();
        entity->u_highlightIndex = node.getHighlightIndex(ctx);
    }

    node.setEntityRange(m_firstEntityIndex, m_asteroidCount);
}

void AsteroidBeltController::createAsteroids(
    const Assets& assets,
    Registry* registry,
    Node& node)
{
    auto& entityRegistry = *registry->m_entityRegistry;

    auto& type = node.m_type;

    const Mesh* mesh = node.m_type->getMesh();
    const auto& volume = mesh->getAABB().getVolume();

    m_firstEntityIndex = registry->m_entityRegistry->addEntityRange(m_asteroidCount);

    for (size_t i = 0; i < m_asteroidCount; i++)
    {
        auto& asteroid = m_asteroids.emplace_back();
        asteroid.m_entityIndex = m_firstEntityIndex + i;

        auto* entity = entityRegistry.updateEntity(asteroid.m_entityIndex, true);
        entity->u_volume = volume;
    }

    initAsteroids(assets, registry, node, m_asteroids);
    calculateVolume(node, m_asteroids);
}

void AsteroidBeltController::initAsteroids(
    const Assets& assets,
    Registry* registry,
    Node& node,
    std::vector<Asteroid>& asteroids)
{
    size_t count = asteroids.size();

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

        auto* entity = registry->m_entityRegistry->updateEntity(asteroid.m_entityIndex, true);
        entity->setObjectID(node.m_objectID);
        entity->u_materialIndex = node.getMaterialIndex();
    }
}

void AsteroidBeltController::rotateAsteroids(
    const RenderContext& ctx,
    Node& node,
    std::vector<Asteroid>& asteroids)
{
    const float elapsed = ctx.m_clock.elapsedSecs;
    const size_t count = asteroids.size();

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
    Node& node,
    std::vector<Asteroid> asteroids)
{
    AABB minmax{ true };

    for (auto&& asteroid : asteroids)
    {
        minmax.minmax(asteroid.m_position);
    }

    node.setAABB(minmax);
}
