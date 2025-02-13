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

#include "material/ImageTexture.h"

#include "model/Node.h"

#include "mesh/LodMesh.h"
#include "mesh/MeshType.h"
#include "mesh/MeshTransform.h"

#include "mesh/MeshSet.h"

#include "physics/PhysicsEngine.h"
#include "physics/HeightMap.h"

#include "engine/PrepareContext.h"
#include "engine/UpdateContext.h"

#include "event/Dispatcher.h"

#include "registry/Registry.h"
#include "registry/NodeRegistry.h"
#include "registry/EntityRegistry.h"
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

    void TerrainGenerator::prepareWT(
        const PrepareContext& ctx,
        Node& container)
    {
        container.m_visible = false;

        const auto& assets = ctx.m_assets;

        m_gridSize = assets.terrainGridSize;

        m_poolSizeU = 4;
        m_poolSizeV = 4;

        auto heightMapId = prepareHeightMap(ctx, container);

        createTiles(ctx, container);
    }

    void TerrainGenerator::updateWT(
        const UpdateContext& ctx,
        const Node& container)
    {
        const auto& state = container.getState();
        if (m_containerMatrixLevel == state.getMatrixLevel()) return;

        updateTiles(ctx, container);
        state.m_dirtySnapshot = true;
        m_containerMatrixLevel = state.getMatrixLevel();
    }

    physics::height_map_id TerrainGenerator::prepareHeightMap(
        const PrepareContext& ctx,
        const Node& container)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;

        bool flipY = false;
        ImageTexture* texture = loadTexture(flipY);
        if (!texture) return 0;
        if (!texture->isValid()) return 0;

        m_heightMapTex = texture;

        auto& pe = physics::PhysicsEngine::get();
        auto heightMapId = pe.registerHeightMap();

        {
            auto* heightMap = pe.modifyHeightMap(heightMapId);
            heightMap->m_origin = &container;
            heightMap->m_verticalRange = m_verticalRange;
            heightMap->m_horizontalScale = m_horizontalScale;

            heightMap->m_worldTileSize = m_worldTileSize;
            heightMap->m_worldSizeU = m_worldTileSize * m_worldTilesU;
            heightMap->m_worldSizeV = m_worldTileSize * m_worldTilesV;

            heightMap->prepare(texture->m_image.get(), true);
        }

        return heightMapId;
    }

    ImageTexture* TerrainGenerator::loadTexture(bool flipY) {
        const auto& texturePath = m_material.resolveTexturePath(m_heightMapFile, false);
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
                false,
                true,
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
        const Node& container)
    {
        return;

        const auto& containerState = container.getState();

        const int step = m_worldTileSize;

        for (size_t idx = 0; idx < m_tileInfos.size(); idx++) {
            const auto& info = m_tileInfos[idx];

            const glm::vec3 pos{ step / 2 + info.m_tileU * step, 0, step / 2 + info.m_tileV * step };

            auto* node = m_nodes[idx].toNode();
            auto& state = node->modifyState();

            state.setPosition(pos);
        }
    }

    void TerrainGenerator::createTiles(
        const PrepareContext& ctx,
        const Node& container)
    {
        const auto& assets = ctx.m_assets;
        auto& registry = ctx.m_registry;
        auto& dispatcher = registry->m_dispatcherWorker;

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

        std::vector<NodeState> states;
        states.reserve(tileCount);

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

            {
                auto& state = states.emplace_back();
                state.setVolume(tileVolume);
                state.setScale(scale);
                state.m_tileIndex = info.m_registeredIndex;

                const glm::vec3 pos{
                    step / 2 + info.m_tileU * step,
                    0,
                    step / 2 + info.m_tileV * step };
                state.setPosition(pos);
            }
        }

        auto typeHandle = createType(registry, container.m_typeHandle);
        {
            // TODO KI just generate primitive mesh
            auto future = ModelRegistry::get().getMeshSet(
                "",
                m_modelsDir,
                TERRAIN_QUAD_MESH_NAME,
                false,
                false);
            auto* meshSet = future.get();
            //meshSet->setAABB(aabb);

            {
                m_material.tilingX = (float)m_worldTilesU;
                m_material.tilingY = (float)m_worldTilesV;
            }

            {
                auto* type = typeHandle.toType();

                type->addMeshSet(*meshSet);

                auto* lodMesh = type->modifyLodMesh(0);

                //lodMesh->m_priority = containerType->m_priority;
                lodMesh->setMaterial(&m_material);
                lodMesh->registerMaterial();

                lodMesh->m_flags.tessellation = true;
                lodMesh->m_drawOptions.m_mode = backend::DrawOptions::Mode::patches;
                lodMesh->m_drawOptions.m_patchVertices = 3;
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

            node->setName("<terrain_tiles>");
            node->m_typeHandle = typeHandle;

            node->modifyState().setVolume(minmax.getVolume());

            m_nodeHandle = handle;
        }

        {
            NodeState state{};
            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body.state = state;
            evt.body.node = {
                .target = m_nodeHandle.toId(),
                .parentId = container.getId(),
            };
            assert(evt.body.node.target > 1);
            dispatcher->send(evt);
        }

        m_nodes.reserve(states.size());
        for (int idx = 0; auto& state : states)
        {
            ki::node_id nodeId{ ki::StringID::nextID() };
            auto handle = pool::NodeHandle::allocate(nodeId);
            auto* node = handle.toNode();
            node->setName(fmt::format("tile-{}", idx));
            node->m_typeHandle = typeHandle;
            m_nodes.push_back(handle);

            event::Event evt { event::Type::node_add };
            evt.blob = std::make_unique<event::BlobData>();
            evt.blob->body.state = state;
            evt.body.node = {
                .target = handle.toId(),
                .parentId = m_nodeHandle.toId(),
            };
            assert(evt.body.node.target > 1);
            dispatcher->send(evt);
            idx++;
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
        type->m_nodeType = NodeType::terrain;

        if (const auto* layer = LayerInfo::findLayer(LAYER_MAIN); layer) {
            type->m_layer = layer->m_index;
        }

        auto& flags = type->m_flags;
        flags = containerType->m_flags;
        flags.invisible = false;
        flags.terrain = true;
        flags.contained = true;

        return typeHandle;
    }
}
