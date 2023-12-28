#include "TerrainGenerator.h"

#include "util/Perlin.h"

#include "asset/Image.h"

#include "asset/AABB.h"
#include "asset/ModelMesh.h"
#include "asset/TerrainMesh.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/MeshType.h"
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
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    m_gridSize = assets.terrainGridSize;

    m_poolSizeU = 4;
    m_poolSizeV = 4;

    prepareHeightMap(assets, registry, container);

    createTiles(assets, registry, container);
}

void TerrainGenerator::update(
    const UpdateContext& ctx,
    Node& container)
{
    if (m_containerMatrixLevel == container.getMatrixLevel()) return;

    updateTiles(ctx, container);
    container.getTransform().m_dirtyEntity = true;
    m_containerMatrixLevel = container.getMatrixLevel();
}

void TerrainGenerator::prepareHeightMap(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
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

    // Make tiles visible
    setActiveRange(m_reservedFirst, m_reservedCount);
}

void TerrainGenerator::createTiles(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    auto* entityRegistry = registry->m_entityRegistry;
    auto* materialRegistry = registry->m_materialRegistry;

    float scale = m_worldTileSize / 2.f;
    float vertMinAABB = 3.f * m_verticalRange[0] / scale;
    float vertMaxAABB = 3.f * m_verticalRange[1] / scale;

    const AABB aabb{
        glm::vec3{ -1.f, vertMinAABB, -1.f },
        glm::vec3{ 1.f, vertMaxAABB, 1.f },
        false
    };

    KI_INFO_OUT(fmt::format("TERRAIN_AABB: minY={}, maxY={}", vertMinAABB, vertMaxAABB));

    auto typeId = createType(registry, container.m_type);
    {
        auto future = registry->m_modelRegistry->getMesh(
            TERRAIN_QUAD_MESH_NAME,
            m_modelsDir);
        auto* mesh = future.get();
        mesh->setAABB(aabb);

        {
            auto type = registry->m_typeRegistry->modifyType(typeId);
            type->setMesh(mesh);
            type->m_drawOptions.patchVertices = 3;
        }
    }

    // NOTE KI must laod textures in the context of *THIS* material
    // NOTE KI only SINGLE material supported
    int materialIndex = -1;
    {
        auto type = registry->m_typeRegistry->modifyType(typeId);
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

    m_reservedCount = m_worldTilesU * m_worldTilesV;
    m_reservedFirst = entityRegistry->registerEntityRange(m_reservedCount);

    m_transforms.reserve(m_reservedCount);

    // Setup initial static values for entity
    int idx = 0;
    for (int v = 0; v < m_worldTilesV; v++) {
        for (int u = 0; u < m_worldTilesU; u++) {
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };
            minmax.minmax(pos);

            const int entityIndex = m_reservedFirst + idx;

            {
                auto& transform = m_transforms.emplace_back();

                transform.setMaterialIndex(materialIndex);
                transform.setVolume(tileVolume);

                transform.m_entityIndex = entityIndex;
            }

            {
                auto* entity = entityRegistry->modifyEntity(entityIndex, false);
                entity->u_tileX = u;
                entity->u_tileY = v;

                entity->u_rangeYmin = m_verticalRange[0];
                entity->u_rangeYmax = m_verticalRange[1];
            }

            idx++;
        }
    }

    {
        const auto type = registry->m_typeRegistry->getType(typeId);
        m_node = new Node(type);
        m_node->setVolume(minmax.getVolume());
        m_node->m_instancer = this;
    }


    {
        event::Event evt { event::Type::node_add };
        evt.body.node = {
            .target = m_node,
            .uuid = {},
            .parentUUID = {},
            .parentId = container.m_id,
        };
        registry->m_dispatcher->send(evt);
    }
}

ki::type_id TerrainGenerator::createType(
    Registry* registry,
    const MeshType* containerType)
{
    auto type = registry->m_typeRegistry->registerType(containerType->m_name);
    type->m_entityType = EntityType::terrain;

    auto& flags = type->m_flags;
    flags = containerType->m_flags;
    flags.invisible = false;
    flags.terrain = true;

    type->m_priority = containerType->m_priority;
    //type->m_script = containerType->m_script;

    // TODO KI *redundant* copy of material
    auto& containerMaterials = containerType->m_materialVBO;
    auto& materialVBO = type->m_materialVBO;

    // NOTE MUST copy *all* data from materials
    auto* material = containerMaterials.getDefaultMaterial();
    if (material) {
        materialVBO.setDefaultMaterial(*material, true, true);
    }
    materialVBO.setMaterials(containerMaterials.getMaterials());

    type->m_program = containerType->m_program;
    type->m_depthProgram = containerType->m_depthProgram;

    return type->m_id;
}
