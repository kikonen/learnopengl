#include "GridGenerator.h"

#include <vector>
#include <random>

#include "util/debug.h"

#include "ki/sid.h"

#include "mesh/MeshType.h"
#include "mesh/Mesh.h"
#include "mesh/MeshTransform.h"

#include "model/Node.h"

#include "event/Dispatcher.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"


GridGenerator::GridGenerator()
{
    m_lightWeight = true;
}

void GridGenerator::prepareWT(
    const PrepareContext& ctx,
    Node& container)
{
    //container.m_visible = false;
    prepareInstances(
        ctx,
        container);
}

void GridGenerator::updateWT(
    const UpdateContext& ctx,
    const Node& container)
{
    auto& containerState = container.getState();
    const auto containerLevel = containerState.getMatrixLevel();
    if (m_containerMatrixLevel == containerLevel) return;

    updateInstances(
        ctx,
        container);

    const auto& parentMatrix = containerState.getModelMatrix();
    for (auto& transform : m_transforms) {
        transform.updateTransform(parentMatrix, m_volume);
    }

    m_containerMatrixLevel = containerLevel;
}

void GridGenerator::updateInstances(
    const UpdateContext& ctx,
    const Node& container)
{
}

void GridGenerator::prepareInstances(
    const PrepareContext& ctx,
    const Node& container)
{
    auto* type = container.m_typeHandle.toType();
    const auto& containerState = container.getState();

    m_volume = containerState.getVolume();

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
    for (int i = 0; i < count; i++) {
        auto& transform = m_transforms.emplace_back();
        transform.setPosition(m_offset);
        transform.setScale(m_scale);
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
    const Node& container,
    std::vector<mesh::MeshTransform>& transforms) const
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
    const Node& container,
    std::vector<mesh::MeshTransform>& transforms) const
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
