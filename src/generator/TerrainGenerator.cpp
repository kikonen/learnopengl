#include "TerrainGenerator.h"

#include <iostream>

#include <fmt/format.h>

#include "ki/sid.h"

#include "pool/NodeHandle.h"

#include "util/Log.h"
#include "util/glm_format.h"
#include "util/Perlin.h"

#include "asset/Image.h"
#include "asset/AABB.h"

#include "model/Node.h"

#include "mesh/ModelMesh.h"
#include "mesh/TerrainMesh.h"
#include "mesh/MeshType.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/MeshTypeRegistry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ModelRegistry.h"

namespace {
    const std::string TERRAIN_QUAD_MESH_NAME{ "quad_terrain" };

}

TerrainGenerator::TerrainGenerator()
    : NodeGenerator()
{
}

void TerrainGenerator::prepare(
    const PrepareContext& ctx,
    Node& container)
{
    m_gridSize = ctx.m_assets.terrainGridSize;

    m_poolSizeU = 4;
    m_poolSizeV = 4;

    auto* heightMap = prepareHeightMap(ctx, container);

    createTiles(ctx, container, heightMap);
    prepareSnapshots(*ctx.m_registry->m_snapshotRegistry);
}

void TerrainGenerator::prepareEntity(
    EntitySSBO& entity,
    uint32_t index)
{
    // TODO KI WT vs RT conflict
    const auto& info = m_tileInfos[index];

    entity.u_tileX = info.m_tileX;
    entity.u_tileY = info.m_tileY;

    entity.u_rangeYmin = m_verticalRange[0];
    entity.u_rangeYmax = m_verticalRange[1];
}


void TerrainGenerator::updateWT(
    const UpdateContext& ctx,
    Node& container)
{
    auto& transform = container.modifyTransform();
    if (m_containerMatrixLevel == transform.getMatrixLevel()) return;

    updateTiles(ctx, container);
    transform.m_dirtySnapshot = true;
    m_containerMatrixLevel = transform.getMatrixLevel();
}

physics::HeightMap* TerrainGenerator::prepareHeightMap(
    const PrepareContext& ctx,
    Node& container)
{
    auto& assets = ctx.m_assets;
    auto& registry = ctx.m_registry;

    const auto& imagePath = m_material.getTexturePath(assets, m_material.map_height);
    KI_INFO(fmt::format("TERRAIN: height={}", imagePath));

    auto image = std::make_unique<Image>(imagePath, false);
    // NOTE KI don't flip, otherwise have to reverse offsets
    int res = image->load();

    auto* pe = registry->m_physicsEngine;
    auto id = pe->registerHeightMap();
    auto* heightMap = pe->getHeightMap(id);

    {
        heightMap->m_origin = &container;
        heightMap->m_verticalRange = m_verticalRange;
        heightMap->m_horizontalScale = m_horizontalScale;

        heightMap->m_worldTileSize = m_worldTileSize;
        heightMap->m_worldSizeU = m_worldTileSize * m_worldTilesU;
        heightMap->m_worldSizeV = m_worldTileSize * m_worldTilesV;

        glm::vec3 min{};
        glm::vec3 max{};
        AABB aabb{ min, max, false };

        heightMap->setAABB(aabb);
    }
    heightMap->prepare(image.get());

    return heightMap;
}

void TerrainGenerator::updateTiles(
    const UpdateContext& ctx,
    Node& container)
{
    const auto& containerTransform = container.getTransform();

    // NOTE scale.y == makes *FLAT* plane
    const glm::vec3 scale{ m_worldTileSize / 2, 1, m_worldTileSize / 2 };
    const int step = m_worldTileSize;

    int idx = 0;
    for (int v = 0; v < m_worldTilesV; v++) {
        for (int u = 0; u < m_worldTilesU; u++) {
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };

            auto& transform = m_transforms[idx];

            transform.setPosition(pos);
            transform.setScale(scale);

            transform.updateModelMatrix(containerTransform);

            idx++;
        }
    }

    m_reservedCount = static_cast<uint32_t>(m_transforms.size());
    setActiveRange(0, m_reservedCount);
}

void TerrainGenerator::createTiles(
    const PrepareContext& ctx,
    Node& container,
    physics::HeightMap* heightMap)
{
    const auto& assets = ctx.m_assets;
    auto& registry = ctx.m_registry;

    auto* entityRegistry = registry->m_entityRegistry;
    auto* materialRegistry = registry->m_materialRegistry;

    const float scale = m_worldTileSize / 2.f;

    const float vertMinAABB = 3.f * m_verticalRange[0] / scale;
    const float vertMaxAABB = 3.f * m_verticalRange[1] / scale;

    const AABB aabb{
        glm::vec3{ -1.f, vertMinAABB, -1.f },
        glm::vec3{ 1.f, vertMaxAABB, 1.f },
        false
    };

    KI_INFO_OUT(fmt::format("TERRAIN_AABB: minY={}, maxY={}", vertMinAABB, vertMaxAABB));

    const auto typeId = createType(registry, container.m_type);
    {
        auto future = registry->m_modelRegistry->getMesh(
            TERRAIN_QUAD_MESH_NAME,
            m_modelsDir);
        auto* mesh = future.get();
        mesh->setAABB(aabb);

        {
            auto* type = registry->m_typeRegistry->modifyType(typeId);
            type->setMesh(mesh);
        }
    }

    // NOTE KI must laod textures in the context of *THIS* material
    // NOTE KI only SINGLE material supported
    int materialIndex = -1;
    {
        auto* type = registry->m_typeRegistry->modifyType(typeId);

        auto& drawOptions = type->modifyDrawOptions();
        drawOptions.m_patchVertices = 3;

        type->modifyMaterials([this, &materialIndex, &materialRegistry, &assets](Material& m) {
            m.tilingX = (float)m_worldTilesU;
            m.tilingY = (float)m_worldTilesV;

            m.loadTextures(assets);

            materialRegistry->registerMaterial(m);
            materialIndex = m.m_registeredIndex;
        });
    }

    const glm::vec4 tileVolume = aabb.getVolume();
    const int step = m_worldTileSize;
    AABB minmax{ true };

    const int tileCount = m_worldTilesU * m_worldTilesV;

    m_transforms.reserve(tileCount);
    m_tileInfos.reserve(tileCount);

    // Setup initial static values for entity
    int idx = 0;
    KI_INFO_OUT(fmt::format("TERRAIN: tilesV={}, tilesU={}", m_worldTilesV, m_worldTilesU));
    for (int v = 0; v < m_worldTilesV; v++) {
        for (int u = 0; u < m_worldTilesU; u++) {
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };

            // TODO KI get height
            {
                glm::vec3 uvPos{
                    pos.x / heightMap->m_worldSizeU,
                    0.f,
                    1.f - pos.z / heightMap->m_worldSizeU };
                const auto height = heightMap->getTerrainHeight(uvPos.x, uvPos.z);
                minmax.minmax({ pos.x, height, pos.z });

                KI_INFO_OUT(fmt::format(
                    "v={}, u={}, pos={}, uvPos={}, height={}, offsetU={}, offsetV={}, min={}, max={}",
                    v, u, pos, uvPos, height, -1200 + v, -900 + u, minmax.m_min, minmax.m_max));
            }

            {
                auto& transform = m_transforms.emplace_back();

                transform.setMaterialIndex(materialIndex);
                transform.setVolume(tileVolume);
            }

            {
                m_tileInfos.emplace_back(u, v);
            }

            idx++;
        }
    }

    // NOTE KI dummy node needed to trigger instancing in container context
    {
        minmax.updateVolume();
        KI_INFO_OUT(fmt::format("TERRAIN: minmax={}", minmax.getVolume()));

        const auto type = registry->m_typeRegistry->getType(typeId);

        auto nodeId = SID("<terrain_tiles>");
        auto handle = pool::NodeHandle::allocate(nodeId);
        auto* node = handle.toNode();
#ifdef _DEBUG
        m_node->m_resolvedSID = "<terrain_tiles>";
#endif
        m_node->m_type = type;

        m_node->modifyTransform().setVolume(minmax.getVolume());
        m_node->m_instancer = this;
    }

    {
        event::Event evt { event::Type::node_add };
        evt.body.node = {
            .target = m_node,
            .id = m_node->getId(),
            .parentId = container.getId(),
        };
        registry->m_dispatcher->send(evt);
    }
}

ki::type_id TerrainGenerator::createType(
    Registry* registry,
    const mesh::MeshType* containerType)
{
    auto type = registry->m_typeRegistry->registerType(containerType->getName());
    type->m_entityType = mesh::EntityType::terrain;

    auto& flags = type->m_flags;
    flags = containerType->m_flags;
    flags.invisible = false;
    flags.terrain = true;
    flags.contained = true;

    type->m_priority = containerType->m_priority;
    type->m_program = containerType->m_program;
    type->m_shadowProgram = containerType->m_shadowProgram;
    type->m_preDepthProgram = containerType->m_preDepthProgram;

    // TODO KI *redundant* copy of material
    {
        auto& containerMaterials = containerType->m_materialVBO;
        auto& materialVBO = type->m_materialVBO;

        // NOTE MUST copy *all* data from materials
        auto* material = containerMaterials->getDefaultMaterial();
        if (material) {
            materialVBO->setDefaultMaterial(*material, true, true);
        }
        materialVBO->setMaterials(containerMaterials->getMaterials());
    }

    return type->getId();
}
