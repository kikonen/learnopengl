#include "GridGenerator.h"

#include <vector>
#include <random>

#include "util/debug.h"
#include "util/thread.h"

#include "ki/sid.h"

#include "mesh/Mesh.h"
#include "mesh/Transform.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "component/definition/PhysicsDefinition.h"

#include "physics/PhysicsSystem.h"
#include "physics/physics_util.h"
#include "physics/Geom.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


GridGenerator::GridGenerator()
{
    m_lightWeight = true;
    m_lightWeightPhysics = true;
}

void GridGenerator::prepareWT(
    const PrepareContext& ctx,
    model::Node& container)
{
    ASSERT_WT();

    //container.m_visible = false;
    prepareInstances(
        ctx,
        container);
}

void GridGenerator::updateWT(
    const UpdateContext& ctx,
    const model::Node& container)
{
    ASSERT_WT();

    const auto& containerState = container.getState();
    const auto containerLevel = containerState.getMatrixLevel();

    // NOTE KI cannot skip for dynamic bounds, physics is assumed to be changing
    if (m_boundsSetupDone && m_staticBounds && m_containerMatrixLevel == containerLevel) return;

    if (m_containerMatrixLevel != containerLevel) {
        m_boundsSetupDone = false;
    }

    updateBounds(ctx, container);
    updateInstances(ctx, container);

    const auto hasPhysics = !m_geometries.empty();
    //auto& physicsSystem = physics::PhysicsSystem::get();

    const auto& parentMatrix = containerState.getModelMatrix();
    for (int i = 0; i < m_transforms.size(); i++) {
        auto& transform = m_transforms[i];
        transform.updateMatrix();
        transform.updateWorldVolume(parentMatrix, m_localVolume);

        if (hasPhysics) {
            const glm::vec3& pos = transform.getWorldPosition();
            const glm::vec3 pivot{ 0.f };
            const auto& rot = transform.getRotation();

            auto& geom = m_geometries[i];
            //geom.setPhysicPosition(transform.getWorldPosition());
            geom.updatePhysic(pivot, pos, rot);
        }
    }

    m_containerMatrixLevel = containerLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    const model::Node& container)
{
}

void GridGenerator::prepareInstances(
    const PrepareContext& ctx,
    const model::Node& container)
{
    const auto& containerState = container.getState();

    m_staticBounds = container.m_typeFlags.staticBounds;
    m_dynamicBounds = container.m_typeFlags.dynamicBounds;

    m_localVolume = containerState.getLocalVolume();

    size_t count = 0;

    switch (m_mode) {
    case GeneratorMode::grid:
        count = m_zCount * m_xCount * m_yCount;
        break;
    case GeneratorMode::random:
        count = m_count > 0 ? m_count : m_zCount * m_xCount * m_yCount;
        break;
    }

    m_transforms.reserve(count);

    if (m_geometryTemplate) {
        m_geometries.reserve(count);
    }

    auto& physicsSystem = physics::PhysicsSystem::get();

    for (int i = 0; i < count; i++) {
        auto& transform = m_transforms.emplace_back();
        transform.setPosition(m_offset);
        transform.setScale(m_scale);

        if (m_geometryTemplate) {
            physics::Object obj;
            obj.m_geom = *m_geometryTemplate;

            m_geometries.push_back(std::move(obj.m_geom));
            auto& geom = m_geometries[m_geometries.size() - 1];

            physicsSystem.registerGeom(geom, glm::vec3{ m_scale });
        }
    }

    switch (m_mode) {
    case GeneratorMode::grid:
        prepareGrid(container, m_transforms);
        break;
    case GeneratorMode::random:
        prepareRandom(container, m_transforms);
        break;
    }
}

void GridGenerator::prepareGrid(
    const model::Node& container,
    std::vector<mesh::Transform>& transforms) const
{
    //const auto& containerState = container.getState();
    int idx = 0;

    for (auto z = 0; z < m_zCount; z++) {
        for (auto y = 0; y < m_yCount; y++) {
            for (auto x = 0; x < m_xCount; x++) {
                auto& transform = transforms[idx];

                glm::vec3 pos{ x * m_xStep, y * m_yStep, z * m_zStep };
                pos += m_offset;

                transform.setPosition(pos);

                idx++;
            }
        }
    }
}

void GridGenerator::prepareRandom(
    const model::Node& container,
    std::vector<mesh::Transform>& transforms) const
{
    const auto count = transforms.size();

    std::random_device devPos;
    std::random_device devScale;
    std::random_device devRot;

    std::mt19937 rngPos(devPos());
    rngPos.seed(m_seed.x);

    std::mt19937 rngScale(devScale());
    rngScale.seed(m_seed.y);

    std::mt19937 rngRot(devRot());
    rngRot.seed(m_seed.z);

    constexpr int RANGE = INT_MAX;
    std::uniform_int_distribution<std::mt19937::result_type> uniform_dist(0, RANGE);

    const float maxX = m_xCount > 0 ? (m_xCount - 1) * m_xStep : 0.f;
    const float maxY = m_yCount > 0 ? (m_yCount - 1) * m_yStep : 0.f;
    const float maxZ = m_zCount > 0 ? (m_zCount - 1) * m_zStep : 0.f;

    for (int idx = 0; idx < count; idx++) {
        auto& transform = transforms[idx];

        {
            glm::uvec3 v{
                uniform_dist(rngPos),
                uniform_dist(rngPos),
                uniform_dist(rngPos)
            };

            glm::vec3 d{
                static_cast<float>(v.x) / (float)RANGE,
                static_cast<float>(v.y) / (float)RANGE,
                static_cast<float>(v.z) / (float)RANGE,
            };

            //KI_INFO_OUT(fmt::format("r={}, v={}, p={}", RANGE, v, d));

            glm::vec3 pos{
                maxX * d.x,
                maxY * d.y,
                maxZ * d.z };
            pos += m_offset;

            transform.setPosition(pos);
        }
        {
            glm::uvec3 v{
                uniform_dist(rngRot),
                uniform_dist(rngRot),
                uniform_dist(rngRot)
            };

            glm::vec3 d{
                static_cast<float>(v.x) / (float)RANGE,
                static_cast<float>(v.y) / (float)RANGE,
                static_cast<float>(v.z) / (float)RANGE,
            };

            float degrees = 360.f * d.y;
            const auto rot = util::axisDegreesToQuat({ 0, 1.f, 0 }, degrees);

            transform.setRotation(rot);
        }
        {
            glm::uvec3 v{
                uniform_dist(rngScale),
                uniform_dist(rngScale),
                uniform_dist(rngScale)
            };

            glm::vec3 d{
                static_cast<float>(v.x) / (float)RANGE,
                static_cast<float>(v.y) / (float)RANGE,
                static_cast<float>(v.z) / (float)RANGE,
            };

            float scale = 0.8f + 0.4f * d.y;
            scale *= m_scale;

            transform.setScale(scale);
        }
    }
}

void GridGenerator::updateBounds(
    const UpdateContext& ctx,
    const model::Node& container)
{
    if (!m_staticBounds && !m_dynamicBounds) return;

    auto& physicsSystem = physics::PhysicsSystem::get();
    if (!physicsSystem.isEnabled()) return;

    if (m_boundsSetupDone) return;

    const auto& containerState = container.getState();
    const auto& containerPos = containerState.getWorldPosition();
    const auto& parentMatrix = containerState.getModelMatrix();

    std::vector<glm::vec3> positions;
    for (auto& transform : m_transforms) {
        transform.updateMatrix();
        transform.updateWorldVolume(parentMatrix, m_localVolume);
        positions.push_back(transform.getWorldPosition());
    }

    const auto& results = physicsSystem.getWorldSurfaceLevels(
        positions,
        m_boundsDir,
        m_boundsMask);

    if (results.empty()) return;

    for (int i = 0; i < m_transforms.size(); i++) {
        const auto& [success, level] = results[i];

        if (success) {
            if (m_staticBounds) {
                m_boundsSetupDone = true;
            }

            auto& transform = m_transforms[i];
            const auto y = level - containerPos.y;
            auto newPos = transform.getPosition();
            newPos.y = y;
            transform.setPosition(newPos);
        }
    }
}
