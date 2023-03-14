#include "TerrainGenerator.h"

#include "util/Perlin.h"

#include "asset/Image.h"

#include "asset/AABB.h"
#include "asset/ModelMesh.h"
#include "asset/TerrainMesh.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "scene/UpdateContext.h"

#include "registry/MeshType.h"
#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/MaterialRegistry.h"


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
    Node& container,
    Node* containerParent)
{
    if (m_containerMatrixLevel == container.getMatrixLevel()) return;

    updateTiles(ctx, container, containerParent);
    m_containerMatrixLevel = container.getMatrixLevel();
}

void TerrainGenerator::prepareHeightMap(
    const Assets& assets,
    Registry* registry,
    Node& container)
{
    const auto& imagePath = m_material.getTexturePath(assets, m_material.map_height);
    KI_INFO(fmt::format("TERRAIN: height={}", imagePath));

    auto image = std::make_unique<Image>(imagePath);
    // NOTE KI don't flip, otherwise have to reverse offsets
    int res = image->load(false);

    auto heightMap = std::make_unique<physics::HeightMap>(std::move(image));
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
    m_heightMap = static_cast<physics::HeightMap*>(registry->m_physicsEngine->registerSurface(std::move(heightMap)));
    m_heightMap->prepare();
}

void TerrainGenerator::updateTiles(
    const UpdateContext& ctx,
    Node& container,
    Node* containerParent)
{
    const auto& containerMatrix = container.getModelMatrix();
    const auto containerLevel = container.getMatrixLevel();

    // NOTE scale.y == makes *FLAT* plane
    const glm::vec3 scale{ m_worldTileSize / 2, 1, m_worldTileSize / 2 };
    const int step = m_worldTileSize;

    int idx = 0;
    for (int v = 0; v < m_worldTilesV; v++) {
        for (int u = 0; u < m_worldTilesU; u++) {
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };

            auto& instance = m_instances[idx];

            instance.setPosition(pos);
            instance.setScale(scale);

            instance.updateModelMatrix(containerMatrix, containerLevel);

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
    auto* entityRegistry = registry->m_entityRegistry.get();
    auto* materialRegistry = registry->m_materialRegistry.get();

    float scale = m_worldTileSize / 2.f;
    float vertMinAABB = 3.f * m_verticalRange[0] / scale;
    float vertMaxAABB = 3.f * m_verticalRange[1] / scale;

    const AABB aabb{
        glm::vec3{ -1.075f, vertMinAABB, -1.075f },
        glm::vec3{ 1.075f, vertMaxAABB, 1.075f },
        true
    };

    KI_INFO_OUT(fmt::format("TERRAIN_AABB: minY={}, maxY={}", vertMinAABB, vertMaxAABB));

    auto type = createType(registry, container.m_type);
    {
        auto future = registry->m_modelRegistry->getMesh(TERRAIN_QUAD_MESH_NAME);
        auto* mesh = future.get();
        mesh->setAABB(aabb);
        type->setMesh(mesh);
        type->m_drawOptions.patchVertices = 3;
    }

    // NOTE KI must laod textures in the context of *THIS* material
    // NOTE KI only SINGLE material supported
    int materialIndex = -1;
    type->modifyMaterials([this, &materialIndex , &materialRegistry, &assets](Material& m) {
        m.tilingX = m_worldTilesU;
        m.tilingY = m_worldTilesV;

        m.loadTextures(assets);

        materialRegistry->add(m);
        materialIndex = m.m_registeredIndex;
    });

    const glm::vec4 tileVolume = aabb.getVolume();
    const int step = m_worldTileSize;
    AABB minmax{ true };

    m_reservedCount = m_worldTilesU * m_worldTilesV;
    m_reservedFirst = entityRegistry->addEntityRange(m_reservedCount);

    m_instances.reserve(m_reservedCount);

    // Setup initial static values for entity
    int idx = 0;
    for (int v = 0; v < m_worldTilesV; v++) {
        for (int u = 0; u < m_worldTilesU; u++) {
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };
            minmax.minmax(pos);

            const int entityIndex = m_reservedFirst + idx;

            {
                auto& instance = m_instances.emplace_back();

                instance.setMaterialIndex(materialIndex);
                instance.setVolume(tileVolume);

                instance.m_entityIndex = entityIndex;
            }

            {
                auto* entity = entityRegistry->updateEntity(entityIndex, false);
                entity->u_tileX = u;
                entity->u_tileY = v;

                entity->u_rangeYmin = m_verticalRange[0];
                entity->u_rangeYmax = m_verticalRange[1];
            }

            idx++;
        }
    }

    {
        m_node = new Node(type);
        m_node->setVolume(minmax.getVolume());
        m_node->m_parentId = container.m_id;
        m_node->m_instancer = this;
        registry->m_nodeRegistry->addNode(m_node);
    }
}

MeshType* TerrainGenerator::createType(
    Registry* registry,
    MeshType* containerType)
{
    auto type = registry->m_typeRegistry->getType(containerType->m_name);
    type->m_entityType = EntityType::terrain;

    auto& flags = type->m_flags;
    flags = containerType->m_flags;
    flags.noDisplay = false;
    flags.invisible = false;

    type->m_priority = containerType->m_priority;
    type->m_script = containerType->m_script;

    // TODO KI *redundant* copy of material
    auto& containerMaterials = containerType->m_materialVBO;
    auto& materialVBO = type->m_materialVBO;

    // NOTE MUST copy *all* data from materials
    materialVBO.m_defaultMaterial = containerMaterials.m_defaultMaterial;
    materialVBO.m_useDefaultMaterial = true;
    materialVBO.m_forceDefaultMaterial = true;
    materialVBO.setMaterials(containerMaterials.getMaterials());

    type->m_program = containerType->m_program;

    return type;
}
