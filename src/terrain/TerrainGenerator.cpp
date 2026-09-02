#include "TerrainGenerator.h"

#include <iostream>

#include <fmt/format.h>

#include "ki/sid.h"

#include "pool/NodeHandle.h"

#include "util/Log.h"
#include "util/glm_format.h"
#include "util/Perlin.h"

#include "asset/Assets.h"
#include "asset/AABB.h"

#include "material/material_util.h"
#include "material/ImageRegistry.h"
#include "material/ImageTexture.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/Transform.h"

#include "mesh/MeshSet.h"

#include "physics/PhysicsSystem.h"
#include "physics/HeightMap.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"

#include "TerrainTileRegistry.h"

namespace {
    const std::string TERRAIN_QUAD_MESH_NAME{ "quad_terrain" };
}

namespace terrain {
    TerrainGenerator::TerrainGenerator()
        : NodeGenerator()
    {
        m_material = Material::createMaterial(BasicMaterial::gold);
        m_lightWeight = true;
        m_lightWeightPhysics = false;
        m_updateDrawables = true;
    }

    TerrainGenerator::~TerrainGenerator() = default;

    void TerrainGenerator::prepareWT(
        const PrepareContext& ctx,
        model::Node& container)
    {
        const auto& assets = ctx.getAssets();

        m_gridSize = assets.terrainGridSize;

        m_poolSizeU = 4;
        m_poolSizeV = 4;

        auto heightMapId = prepareHeightMap(ctx, container);

        createTiles(ctx, container);
        markDirty({ 0, m_transforms.size() });
    }

    void TerrainGenerator::updateWT(
        const UpdateContext& ctx,
        const model::Node& container)
    {
        const auto& state = container.getState();
        if (m_containerMatrixLevel == state.getMatrixLevel()) return;

        updateTiles(ctx, container);
        markDirty({ 0, m_transforms.size() });

        state.m_dirtySnapshot = true;
        m_containerMatrixLevel = state.getMatrixLevel();
    }

    physics::height_map_id TerrainGenerator::prepareHeightMap(
        const PrepareContext& ctx,
        const model::Node& container)
    {
        const auto& assets = ctx.getAssets();
        auto* registry = ctx.getRegistry();

        bool flipY = false;
        auto texture = loadTexture(flipY);
        if (!texture) return 0;
        if (!texture->isValid()) return 0;

        m_heightMapTex = texture;

        auto& physicsSystem = physics::PhysicsSystem::get();
        auto heightMapId = physicsSystem.registerHeightMap();

        {
            auto* heightMap = physicsSystem.modifyHeightMap(heightMapId);
            heightMap->m_origin = &container;
            heightMap->m_verticalRange = m_verticalRange;
            heightMap->m_horizontalScale = m_horizontalScale;

            heightMap->m_worldTileSize = m_worldTileSize;
            heightMap->m_worldSizeU = m_worldTileSize * m_worldTilesU;
            heightMap->m_worldSizeV = m_worldTileSize * m_worldTilesV;

            heightMap->prepare(texture->getImage(), true);
        }

        return heightMapId;
    }

    void TerrainGenerator::setMaterial(const util::Ref<Material>& src) noexcept
    {
        if (!src) {
            m_material = Material::createMaterial(BasicMaterial::gold);
            return;
        }

        if (!m_material) {
            m_material = util::Ref<Material>::create();
        }
        *m_material = *src;
    }

    util::Ref<ImageTexture> TerrainGenerator::loadTexture(bool flipY) {
        if (!m_material) return nullptr;

        const auto& texturePath = material::resolveTexturePath(m_heightMapFile);

        KI_INFO(fmt::format("TEX::TERRAIN: valid={}, texture={}",
            texturePath.valid, texturePath.path));

        if (!texturePath.valid) return nullptr;

        {
            material::TextureSpec spec;
            spec.wrap = material::WrapMode::clamp_to_edge;
            spec.maxMipMapLevels = 1;

            auto future = ImageRegistry::get().getTexture(
                texturePath.name,
                texturePath.path,
                true,
                false,
                false,
                true,
                material::TextureType::map_custom_1,
                spec);

            future.wait();

            return future.valid() ? future.get() : nullptr;
        }
    }

    void TerrainGenerator::updateTiles(
        const UpdateContext& ctx,
        const model::Node& container)
    {
        const auto& containerState = container.getState();

        //const int step = m_worldTileSize;

        //for (size_t idx = 0; idx < m_tileInfos.size(); idx++) {
        //    const auto& info = m_tileInfos[idx];

        //    const glm::vec3 pos{ step / 2 + info.m_tileU * step, 0, step / 2 + info.m_tileV * step };

        //    auto* node = m_nodes[idx].toNode();
        //    auto& state = node->modifyState();

        //    state.setPosition(pos);
        //}

        {
            const auto& parentMatrix = containerState.getModelMatrix();
            for (auto& transform : m_transforms) {
                transform.updateMatrix();
                transform.updateWorldVolume(parentMatrix, m_localVolume);
            }
        }
    }

    void TerrainGenerator::createTiles(
        const PrepareContext& ctx,
        const model::Node& container)
    {
        const auto& assets = ctx.getAssets();
        auto* registry = ctx.getRegistry();
        const auto& dispatcherWorker = registry->m_dispatcherWorker;

        auto& entityRegistry = EntityRegistry::get();

        // NOTE scale.y == makes *FLAT* plane
        const glm::vec3 scale{ m_worldTileSize / 2.f, 1, m_worldTileSize / 2.f };

        //const float scale = m_worldTileSize / 2.f;
        //const float vertMinAABB = 3.f * m_verticalRange[0] / scale.x;
        //const float vertMaxAABB = 3.f * m_verticalRange[1] / scale.z;
        {
            const float vertMinAABB = m_verticalRange[0];
            const float vertMaxAABB = m_verticalRange[1];
            KI_INFO_OUT(fmt::format("TERRAIN_AABB: minY={}, maxY={}", vertMinAABB, vertMaxAABB));
        }

        {
            // side extent
            const auto& ext = 0.5f;
            const auto& planeRadius = std::sqrt(ext * ext + ext * ext);
            const auto& cubeRadius = std::sqrt(planeRadius * planeRadius + ext * ext);

            AABB aabb{
                glm::vec3{ 0.f },
                glm::vec3{ cubeRadius }
            };
            m_localVolume = aabb.toLocalVolume();
        }

        const int tileCount = m_worldTilesU * m_worldTilesV;

        m_tileInfos.reserve(tileCount);

        // Setup initial static values for entity
        KI_INFO_OUT(fmt::format("TERRAIN: tilesV={}, tilesU={}", m_worldTilesV, m_worldTilesU));
        {
            auto& ttr = TerrainTileRegistry::get();

            for (int v = 0; v < m_worldTilesV; v++) {
                for (int u = 0; u < m_worldTilesU; u++) {
                    auto& info = m_tileInfos.emplace_back(u, v);

                    info.u_rangeYmin = m_verticalRange[0];
                    info.u_rangeYmax = m_verticalRange[1];

                    info.m_heightMapTex = m_heightMapTex;

                    ttr.addTile(info);
                }
            }
        }

        const int step = m_worldTileSize;

        m_transforms.reserve(tileCount);
        for (size_t idx = 0; idx < m_tileInfos.size(); idx++) {
            const auto& info = m_tileInfos[idx];
            const auto u = info.m_tileU;
            const auto v = info.m_tileV;
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };

            {
                auto& transform = m_transforms.emplace_back();
                transform.setScale(scale);
                transform.setData(info.m_registeredIndex);

                const glm::vec3 pos{
                    step / 2 + info.m_tileU * step,
                    0,
                    step / 2 + info.m_tileV * step };
                transform.setPosition(pos);
            }
        }
    }
}
