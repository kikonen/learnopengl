#include "AsteroidBeltGenerator.h"

#include <algorithm>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "asset/Mesh.h"

#include "model/Node.h"

#include "engine/UpdateContext.h"
#include "render/Batch.h"

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
    const UpdateContext& ctx,
    Node& container)
{
    const int parentLevel = container.getParent()->getMatrixLevel();
    const bool rotate = m_updateIndex% m_updateStep == 0 || parentLevel != m_containerMatrixLevel;

    if (rotate) {
        updateAsteroids(ctx, container, rotate);
    }

    m_updateIndex++;
    m_containerMatrixLevel = parentLevel;
}

void AsteroidBeltGenerator::updateAsteroids(
    const UpdateContext& ctx,
    Node& container,
    bool rotate)
{
    auto* registry = ctx.m_registry;

    const auto& parentInstance = container.getParent()->getInstance();

    if (rotate) {
        rotateAsteroids(ctx, container);
    }

    for (auto& instance : m_instances)
    {
        instance.updateModelMatrix(parentInstance);
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

        asteroid.m_entityIndex = static_cast<int>(m_reservedFirst + i);

        asteroid.setMaterialIndex(container.m_type->getMaterialIndex());
        asteroid.setVolume(volume);

        asteroid.setId(containerInstance.getId());
        asteroid.setFlags(containerInstance.getFlags());

        asteroid.setId(container.m_id);
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
    srand(static_cast<unsigned int>(glfwGetTime()));

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
            asteroid.setDegreesRotation({ rand() % 360, rand() % 360, rand() % 360 });
        }

        {
            // 3. make asteroids to rotate slowly
            float velocity = (rand() % 200) / 20000.0f + 0.0005f;
            physics.m_angularVelocity = velocity;
        }
    }
}

void AsteroidBeltGenerator::rotateAsteroids(
    const UpdateContext& ctx,
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
