#include "AsteroidBeltGenerator.h"

#include <algorithm>
#include <execution>
#include <fmt/format.h>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "ki/sid.h"

#include "mesh/LodMesh.h"
#include "mesh/Transform.h"

#include "model/Node.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"


namespace {
    bool done = false;

    constexpr int STRIDES = 3;
}

AsteroidBeltGenerator::AsteroidBeltGenerator(int asteroidCount)
    : m_asteroidCount(asteroidCount),
    m_radius(70.0),
    m_modifier(20.5f),
    m_updateStep(1)
{
    m_lightWeight = true;
    m_lightWeightPhysics = true;
}

void AsteroidBeltGenerator::prepareWT(
    const PrepareContext& ctx,
    model::Node& container)
{
    NodeGenerator::prepareWT(ctx, container);

    //container.m_visible = false;

    createAsteroids(ctx, container);
}

void AsteroidBeltGenerator::updateWT(
    const UpdateContext& ctx,
    const model::Node& container)
{
    const auto containerLevel = container.getState().getMatrixLevel();
    const auto parentChanged = containerLevel != m_containerMatrixLevel;
    const bool needUpdate = (m_updateIndex % m_updateStep) == 0;

    if (needUpdate) {
        m_updateIndex = 0;
        updateAsteroids(ctx, container, needUpdate);
    }

    if (parentChanged || needUpdate) {
        //auto& parentMatrix = container.getParent()->getState().getModelMatrix();
        const auto& parentMatrix = container.getState().getModelMatrix();

        auto fn = [this, &parentMatrix, parentChanged](const auto idx) {
            if (!parentChanged && (idx % STRIDES) != m_strideIndex) return;
            //if (m_strideIndex != 1) continue;
            m_transforms[idx].updateTransform(parentMatrix, m_volume);
        };

        std::for_each(
            std::execution::par_unseq,
            m_indeces.cbegin(),
            m_indeces.cend(),
            fn);
    }

    m_strideIndex = (m_strideIndex + 1) % STRIDES;
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
        m_volume = containerState.getVolume();
    }

    m_physics.reserve(m_asteroidCount);
    m_transforms.reserve(m_asteroidCount);
    m_indeces.reserve(m_asteroidCount);

    for (size_t i = 0; i < m_asteroidCount; i++)
    {
        m_physics.emplace_back();
        m_transforms.emplace_back();
        m_indeces.push_back(static_cast<uint32_t>(i));
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
    const float elapsed = ctx.m_clock.elapsedSecs;

    auto fn = [this, elapsed](const auto idx) {
        if ((idx % STRIDES) != m_strideIndex) return;
        //if (m_strideIndex != 1) return;

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
    };

    std::for_each(
        std::execution::par_unseq,
        m_indeces.cbegin(),
        m_indeces.cend(),
        fn);
}
