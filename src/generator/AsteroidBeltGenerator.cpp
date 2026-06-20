#include "AsteroidBeltGenerator.h"

#include <algorithm>
#include <execution>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/sid.h"

#include "util/thread.h"

#include "mesh/LodMesh.h"
#include "mesh/Transform.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"


namespace {
    bool done = false;

    constexpr int STRIDES = 20;
}

AsteroidBeltGenerator::AsteroidBeltGenerator(int asteroidCount)
    : m_asteroidCount(asteroidCount),
    m_radius(70.0),
    m_modifier(20.5f),
    m_updateStep(1)
{
    m_lightWeight = true;
    m_lightWeightPhysics = true;
    m_updateDrawables = true;
}

void AsteroidBeltGenerator::prepareWT(
    const PrepareContext& ctx,
    model::Node& container)
{
    ASSERT_WT();

    NodeGenerator::prepareWT(ctx, container);

    //container.m_visible = false;

    createAsteroids(ctx, container);

    markDirty({ 0, m_transforms.size() });
}

void AsteroidBeltGenerator::updateWT(
    const UpdateContext& ctx,
    const model::Node& container)
{
    ASSERT_WT();

    const auto containerLevel = container.getState().getMatrixLevel();
    const auto parentChanged = containerLevel != m_containerMatrixLevel;
    const bool needUpdate = (m_updateIndex % m_updateStep) == 0;

    if (needUpdate) {
        m_updateIndex = 0;
        updateAsteroids(ctx, container, needUpdate);
    }

    if (needUpdate) {
        const auto& parentMatrix = container.getState().getModelMatrix();

        const auto stride = resolveStride();
        for (size_t idx = stride.offset; idx < stride.offset + stride.size; idx++) {
            auto& transform = m_transforms[idx];
            transform.updateMatrix();
            transform.updateWorldVolume(parentMatrix, m_localVolume);
        }
        markDirty({ stride.offset, stride.size });

        m_strideIndex = (m_strideIndex + 1) % STRIDES;
    }

    m_updateIndex++;
    m_containerMatrixLevel = containerLevel;
}

void AsteroidBeltGenerator::updateAsteroids(
    const UpdateContext& ctx,
    const model::Node& container,
    bool rotate)
{
    if (rotate) {
        rotateAsteroids(ctx, container);
    }
}

void AsteroidBeltGenerator::createAsteroids(
    const PrepareContext& ctx,
    const model::Node& container)
{
    {
        const auto& containerState = container.getState();
        m_localVolume = containerState.getLocalVolume();
    }

    m_physics.reserve(m_asteroidCount);
    m_transforms.reserve(m_asteroidCount);
    m_transformIndeces.reserve(m_asteroidCount);

    for (int i = 0; i < m_asteroidCount; i++)
    {
        m_physics.emplace_back();
        m_transforms.emplace_back();
        m_transformIndeces.push_back(static_cast<uint32_t>(i));
    }

    initAsteroids(ctx, container, m_transforms);
}

void AsteroidBeltGenerator::initAsteroids(
    const PrepareContext& ctx,
    const model::Node& container,
    std::vector<mesh::Transform>& transforms)
{
    // initialize random seed
    auto ts = duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    );
    srand(static_cast<unsigned int>(ts.count()));

    const size_t count = transforms.size();

    const glm::vec3 center{ 0.f };
    const float radius = m_radius;
    const float modifier = m_modifier;

    for (size_t i = 0; i < count; i++)
    {
        auto& asteroid = m_transforms[i];
        auto& physics = m_physics[i];

        {
            // 1. translation: displace along circle with 'radius' in range [-offset, offset]
            float angle = (float)i / (float)count * 360.0f;

            float displacement = (rand() % (int)(2 * modifier * 100)) / 100.0f - modifier;
            float x = sin(angle) * radius + displacement;

            displacement = (rand() % (int)(2 * modifier * 100)) / 100.0f - modifier;
            float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z

            displacement = (rand() % (int)(2 * modifier * 100)) / 100.0f - modifier;
            float z = cos(angle) * radius + displacement;

            asteroid.setPosition({x, y, z});
        }

        {
            // 2. scale: scale between 0.05 and 0.25f
            float scale = (rand() % 20) / 100.0f + 0.05f;
            asteroid.setScale(scale);
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
    const model::Node& container)
{
    const float elapsed = ctx.getClock().elapsedSecs;

    {
        const auto stride = resolveStride();

        for (size_t idx = stride.offset; idx < stride.offset + stride.size; idx++) {
            auto& asteroid = m_transforms[idx];
            auto& physics = m_physics[idx];

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
}

util::BufferReference AsteroidBeltGenerator::resolveStride()
{
    const size_t count = m_transforms.size() / STRIDES;
    const size_t offset = m_strideIndex * count;
    // must match the stride window used in updateWT so positions and the
    // matrices/world volumes derived from them stay in lockstep
    const size_t limit = (m_strideIndex == STRIDES - 1)
        ? m_transforms.size()
        : std::min(m_transforms.size(), offset + count);

    return { offset, limit - offset };
}
