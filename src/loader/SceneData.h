#pragma once

#include <string>
#include <vector>
#include <tuple>
#include <memory>

struct Material;

namespace loader {
    class DocNode;

    struct MetaData;
    struct RootData;
    struct SkyboxData;
    struct ScriptSystemData;
    struct NodeTypeData;
    struct NodeData;
    struct CompositeData;
    struct DecalData;
    struct ParticleData;
    struct MaterialUpdaterData;

    struct SceneData {
        std::unique_ptr<MetaData> m_meta;
        std::unique_ptr<SkyboxData> m_skybox;

        std::unique_ptr<RootData> m_root;
        std::unique_ptr<ScriptSystemData> m_scriptSystemData;

        std::vector<NodeTypeData> m_nodeTypes;
        std::vector<NodeData> m_nodes;
        std::vector<CompositeData> m_composites;

        std::unique_ptr<Material> m_defaultMaterial;

        std::vector<ParticleData> m_particles;
        std::vector<DecalData> m_decals;
        std::vector<MaterialUpdaterData> m_materialUpdaters;

        std::vector<std::pair<std::string, loader::DocNode>> m_includeFiles;

        SceneData();
        ~SceneData();

        const loader::DocNode* findInclude(
            const std::string& filePath);
    };
}
