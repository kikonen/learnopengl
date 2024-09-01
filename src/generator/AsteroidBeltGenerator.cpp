#include "AsteroidBeltGenerator.h"

#include <algorithm>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "event/Dispatcher.h"

#include "mesh/Mesh.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"
#include "render/Batch.h"

#include "registry/Registry.h"


namespace {
    bool done = false;
}

AsteroidBeltGenerator::AsteroidBeltGenerator(int asteroidCount)
    : m_asteroidCount(asteroidCount),
    m_radius(70.0),
    m_offset(20.5f),
    m_updateStep(3)
{
}

void AsteroidBeltGenerator::prepareWT(
    const PrepareContext& ctx,
    Node& container)
{
    NodeGenerator::prepareWT(ctx, container);

    createAsteroids(ctx, container);
}

void AsteroidBeltGenerator::updateWT(
    const UpdateContext& ctx,
    const Node& container)
{
    if (done) return;
    const auto parentLevel = container.getState().getMatrixLevel();
    const bool rotate = m_updateIndex% m_updateStep == 0 || parentLevel != m_containerMatrixLevel;

    if (rotate) {
        updateAsteroids(ctx, container, rotate);
        auto& state = container.getState();
        state.m_dirtySnapshot = true;
    }

    //done = true;
    m_updateIndex++;
    m_containerMatrixLevel = parentLevel;
}

void AsteroidBeltGenerator::updateAsteroids(
    const UpdateContext& ctx,
    const Node& container,
    bool rotate)
{
    if (rotate) {
        rotateAsteroids(ctx, container);
    }
}

void AsteroidBeltGenerator::createAsteroids(
    const PrepareContext& ctx,
    const Node& container)
{
    auto& registry = ctx.m_registry;

    auto* type = container.m_typeHandle.toType();

    auto* lodMesh = type->getLodMesh(0);
    const auto* mesh = lodMesh->m_mesh;
    const auto& volume = mesh->calculateAABB(glm::mat4{ 1.f }).getVolume();

    auto& containerState = container.getState();

    std::vector<NodeState> states;
    states.reserve(m_asteroidCount);

    for (size_t i = 0; i < m_asteroidCount; i++)
    {
        m_physics.emplace_back();

        auto& asteroid = states.emplace_back();
        asteroid.m_flags = type->resolveEntityFlags();
        asteroid.setVolume(volume);
    }

    initAsteroids(ctx, container, states);
}

void AsteroidBeltGenerator::initAsteroids(
    const PrepareContext& ctx,
    const Node& container,
    std::vector<NodeState>& states)
{
    // initialize random seed
    auto ts = duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    srand(static_cast<unsigned int>(ts.count()));

    const size_t count = states.size();

    const glm::vec3 center{ 0.f };
    const float radius = m_radius;
    const float offset = m_offset;

    for (size_t i = 0; i < count; i++)
    {
        auto& asteroid = states[i];
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
            // 4. make asteroids to rotate center slowly
            float velocity = (rand() % 200) / 20000.0f + 0.0005f;
            physics.m_velocity = velocity;
        }

        {
            // 5. each asteroid slow rotates its' own axis
            glm::vec3 axis({ 100 - rand() % 200, 100 - rand() % 200, 100 - rand() % 200 });
            float degrees = (100 - rand() % 200) / 1.f;

            physics.m_axis = glm::normalize(axis);
            physics.m_angularRotation = glm::radians(degrees);
        }
    }
}

void AsteroidBeltGenerator::rotateAsteroids(
    const UpdateContext& ctx,
    const Node& container)
{
    const float elapsed = ctx.m_clock.elapsedSecs;
    const size_t count = m_nodes.size();

    for (size_t i = 0; i < count; i++)
    {
        auto* node = m_nodes[i].toNode();
        auto& asteroid = node->modifyState();
        auto& physics = m_physics[i];

        {
            float angle = physics.m_velocity * elapsed;
            auto mat = glm::toMat4(glm::quat(glm::vec3(0.f, angle, 0.f)));
            asteroid.setPosition(mat * glm::vec4(asteroid.getPosition(), 1.f));
        }

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        {
            auto rot = util::axisRadiansToQuat(physics.m_axis, physics.m_angularRotation * elapsed);
            asteroid.adjustRotation(rot);
        }
    }
}
