#include "AsteroidBeltGenerator.h"

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


AsteroidBeltGenerator::AsteroidBeltGenerator(int asteroidCount)
    : m_asteroidCount(asteroidCount),
    m_radius(70.0),
    m_offset(20.5f),
    m_updateStep(3)
{
}

void AsteroidBeltGenerator::prepare(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    NodeGenerator::prepare(assets, registry, container);

    createAsteroids(assets, registry, container);
}

void AsteroidBeltGenerator::update(
    const RenderContext& ctx,
    Node& container,
    Node* containerParent)
{
    const bool rotate = m_updateIndex% m_updateStep == 0 || container.getMatrixLevel() != m_nodeMatrixLevel;

    if (rotate) {
        updateAsteroids(ctx, container, containerParent, rotate);
    }

    m_updateIndex++;
    m_nodeMatrixLevel = container.getMatrixLevel();
}

void AsteroidBeltGenerator::updateAsteroids(
    const RenderContext& ctx,
    Node& container,
    Node* containerParent,
    bool rotate)
{
    auto* registry = ctx.m_registry;

    const auto& containerMatrix = containerParent->getModelMatrix();
    const auto containerLevel = containerParent->getMatrixLevel();

    if (rotate) {
        rotateAsteroids(ctx, container);
    }

    for (auto& instance : m_instances)
    {
        instance.updateModelMatrix(containerMatrix, containerLevel);
    }

    setActiveRange(m_reservedFirst, m_reservedCount);
    container.m_instancer = this;
}

void AsteroidBeltGenerator::createAsteroids(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    auto& type = container.m_type;

    const Mesh* mesh = container.m_type->getMesh();
    const auto& volume = mesh->getAABB().getVolume();

    const auto& containerInstance = container.getInstance();

    m_reservedCount = m_asteroidCount;
    m_reservedFirst = registry->m_entityRegistry->addEntityRange(m_reservedCount);

    for (size_t i = 0; i < m_asteroidCount; i++)
    {
        m_physics.emplace_back();

        auto& asteroid = m_instances.emplace_back();

        asteroid.m_entityIndex = m_reservedFirst + i;

        asteroid.setMaterialIndex(container.getMaterialIndex());
        asteroid.setVolume(volume);

        asteroid.setObjectID(containerInstance.getObjectID());
        asteroid.setFlags(containerInstance.getFlags());

        asteroid.setObjectID(container.m_objectID);
    }

    initAsteroids(assets, registry, container);
    container.setVolume(calculateVolume());
}

void AsteroidBeltGenerator::initAsteroids(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    // initialize random seed
    srand(glfwGetTime());

    const size_t count = m_instances.size();

    const glm::vec3 center{ 0.f };
    const float radius = m_radius;
    const float offset = m_offset;

    for (size_t i = 0; i < count; i++)
    {
        auto& asteroid = m_instances[i];
        auto& physics = m_physics[i];

        {
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)count * 360.0f;

            float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float x = sin(angle) * radius + displacement;

            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z

            displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
            float z = cos(angle) * radius + displacement;

            asteroid.setPosition({x, y, z});
        }

        {
            // 2. scale: scale between 0.05 and 0.25f
            float scale = (rand() % 20) / 100.0f + 0.05f;
            asteroid.setScale({ scale, scale, scale });
        }

        {
            // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
            asteroid.setRotation({ rand() % 360, rand() % 360, rand() % 360 });
        }

        {
            // 3. make asteroids to rotate slowly
            float velocity = (rand() % 200) / 20000.0f + 0.0005f;
            physics.m_angularVelocity = velocity;
        }
    }
}

void AsteroidBeltGenerator::rotateAsteroids(
    const RenderContext& ctx,
    Node& container)
{
    const float elapsed = ctx.m_clock.elapsedSecs;
    const size_t count = m_instances.size();

    for (size_t i = 0; i < count; i++)
    {
        auto& asteroid = m_instances[i];
        auto& physics = m_physics[i];

        glm::mat4 modelMat{ 1.f };
        float angle = physics.m_angularVelocity * elapsed;

        auto mat = glm::toMat4(glm::quat(glm::vec3(0.f, angle, 0.f)));
        asteroid.setPosition(mat * glm::vec4(asteroid.getPosition(), 1.f));
    }
}
