#include "TerrainGenerator.h"

#include <iostream>

#include <fmt/format.h>

#include "ki/sid.h"

#include "pool/NodeHandle.h"

#include "util/Log.h"
#include "util/glm_format.h"
#include "util/Perlin.h"

#include "asset/Assets.h"
#include "asset/ImageTexture.h"
#include "asset/AABB.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"

#include "mesh/MeshSet.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
#include "registry/MaterialRegistry.h"
#include "registry/ModelRegistry.h"

#include "TerrainTileRegistry.h"

namespace {
    const std::string TERRAIN_QUAD_MESH_NAME{ "quad_terrain" };

}

namespace terrain {

    TerrainGenerator::TerrainGenerator()
        : NodeGenerator()
    {
        m_material = Material::createMaterial(BasicMaterial::gold);
    }

    void TerrainGenerator::prepare(
        const PrepareContext& ctx,
        Node& container)
    {
        const auto& assets = ctx.m_assets;

        m_gridSize = assets.terrainGridSize;

        m_poolSizeU = 4;
        m_poolSizeV = 4;

        auto* heightMap = prepareHeightMap(ctx, container);

        createTiles(ctx, container, heightMap);
        prepareSnapshots(*ctx.m_registry->m_workerSnapshotRegistry);
    }

    void TerrainGenerator::prepareEntity(
        EntitySSBO& entity,
        uint32_t index)
    {
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
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

        ImageTexture* texture = loadTexture();
        if (!texture) return nullptr;
        if (!texture->isValid()) return nullptr;

        m_heightMapTex = texture;

        auto& pe = physics::PhysicsEngine::get();
        auto id = pe.registerHeightMap();
        auto* heightMap = pe.getHeightMap(id);

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
        heightMap->prepare(texture->m_image.get(), true);

        return heightMap;
    }

    ImageTexture* TerrainGenerator::loadTexture() {
        const auto& texturePath = m_material.getTexturePath(m_heightMapFile);
        KI_INFO(fmt::format("TERRAIN: height={}", texturePath));

        //auto image = std::make_unique<Image>(imagePath, false);
        //// NOTE KI don't flip, otherwise have to reverse offsets
        //int res = image->load();

        {
            TextureSpec spec;
            spec.wrapS = GL_CLAMP_TO_EDGE;
            spec.wrapT = GL_CLAMP_TO_EDGE;
            spec.mipMapLevels = 1;

            auto future = ImageTexture::getTexture(
                texturePath,
                texturePath,
                false,
                spec);

            future.wait();

            ImageTexture* texture = { nullptr };
            if (future.valid()) {
                texture = future.get();
            }
            return texture;
        }
    }

    void TerrainGenerator::updateTiles(
        const UpdateContext& ctx,
        Node& container)
    {
        const auto& containerTransform = container.getTransform();

        const int step = m_worldTileSize;

        for (size_t idx = 0; idx < m_tileInfos.size(); idx++) {
            const auto& info = m_tileInfos[idx];

            const glm::vec3 pos{ step / 2 + info.m_tileU * step, 0, step / 2 + info.m_tileV * step };

            auto& transform = m_transforms[idx];

            transform.setPosition(pos);

            transform.updateModelMatrix(containerTransform);
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

        const AABB aabb{
            glm::vec3{ -1.f, 1, -1.f },
            glm::vec3{ 1.f, 1, 1.f },
            false
        };

        const int tileCount = m_worldTilesU * m_worldTilesV;

        m_transforms.reserve(tileCount);
        m_tileInfos.reserve(tileCount);

        // Setup initial static values for entity
        KI_INFO_OUT(fmt::format("TERRAIN: tilesV={}, tilesU={}", m_worldTilesV, m_worldTilesU));
        {
            auto& ttr = TerrrainTileRegistry::get();

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

        const glm::vec4 tileVolume = aabb.getVolume();
        const int step = m_worldTileSize;
        AABB minmax{ true };

        const int worldSizeU = m_worldTileSize * m_worldTilesU;
        const int worldSizeV = m_worldTileSize * m_worldTilesV;

        for (size_t idx = 0; idx < m_tileInfos.size(); idx++) {
            const auto& info = m_tileInfos[idx];
            const auto u = info.m_tileU;
            const auto v = info.m_tileV;
            const glm::vec3 pos{ step / 2 + u * step, 0, step / 2 + v * step };

            // TODO KI get height
            {
                glm::vec3 uvPos{
                    pos.x / worldSizeU,
                    0.f,
                    1.f - pos.z / worldSizeV };
                const auto height = heightMap ? heightMap->getTerrainHeight(uvPos.x, uvPos.z) : 0.f;
                minmax.minmax({ pos.x, height, pos.z });

                KI_INFO_OUT(fmt::format(
                    "u={}, v={}, pos={}, uvPos={}, height={}, offsetU={}, offsetV={}, min={}, max={}",
                    u, v, pos, uvPos, height, -1200 + v, -900 + u, minmax.m_min, minmax.m_max));
            }

            {
                auto& transform = m_transforms.emplace_back();
                transform.setVolume(tileVolume);
                transform.setScale(scale);
                transform.m_shapeIndex = info.m_registeredIndex;
            }
        }

        auto typeHandle = createType(registry, container.m_typeHandle);
        {
            auto future = ModelRegistry::get().getMeshSet(
                m_modelsDir,
                TERRAIN_QUAD_MESH_NAME);
            auto* meshSet = future.get();
            //meshSet->setAABB(aabb);

            {
                m_material.tilingX = (float)m_worldTilesU;
                m_material.tilingY = (float)m_worldTilesV;
            }

            {
                auto* type = typeHandle.toType();

                type->addMeshSet(*meshSet, 0);

                auto* lodMesh = type->modifyLodMesh(0);
                lodMesh->m_material = m_material;

                lodMesh->registerMaterials();
            }
        }

        // NOTE KI dummy node needed to trigger instancing in container context
        {
            minmax.updateVolume();
            KI_INFO_OUT(fmt::format("TERRAIN: minmax={}", minmax.getVolume()));

            auto nodeId = SID("<terrain_tiles>");
            auto handle = pool::NodeHandle::allocate(nodeId);
            auto* node = handle.toNode();
            assert(node);
#ifdef _DEBUG
            node->m_resolvedSID = "<terrain_tiles>";
#endif
            node->m_typeHandle = typeHandle;

            node->modifyTransform().setVolume(minmax.getVolume());
            node->m_instancer = this;

            m_nodeHandle = handle;
        }

        {
            event::Event evt { event::Type::node_add };
            evt.body.node = {
                .target = m_nodeHandle.toId(),
                .parentId = container.getId(),
            };
            registry->m_dispatcherWorker->send(evt);
        }
    }

    pool::TypeHandle TerrainGenerator::createType(
        Registry* registry,
        pool::TypeHandle containerTypeHandle)
    {
        auto* containerType = containerTypeHandle.toType();
        auto typeHandle = pool::TypeHandle::allocate();

        auto* type = typeHandle.toType();
        type->setName(containerType->getName());
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

        auto& drawOptions = type->modifyDrawOptions();
        drawOptions.m_patchVertices = 3;

        return typeHandle;
    }
}
