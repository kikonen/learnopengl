#pragma once

#include <string>
#include <memory>

#include "BaseLoader.h"

namespace model
{
    class NodeType;
}

struct Material;
struct TypeFlags;

namespace animation
{
    struct Rig;
}

namespace pool {
    struct TypeHandle;
}

namespace mesh {
    struct MeshFlags;
    class Mesh;
    struct LodMesh;
}

namespace loader {
    struct NodeTypeData;
    struct CompositeData;
    struct MaterialData;
    struct ScriptSystemData;
    struct TextData;
    struct MeshData;
    struct LodData;
    struct AnimationData;
    struct MaterialData;
    struct DecalData;
    struct MaterialUpdaterData;
    struct ParticleData;

    struct FlagContainer;

    class NodeTypeBuilder
    {
    public:
        NodeTypeBuilder(
            const std::shared_ptr<Context>& ctx,
            const std::shared_ptr<Loaders>& loaders);

        ~NodeTypeBuilder();

        void createTypes(
            const std::vector<NodeTypeData>& types,
            const std::vector<CompositeData>& composites,
            const std::vector<ParticleData>& particles);

        pool::TypeHandle createType(
            const NodeTypeData& typeData,
            const std::vector<CompositeData>& composites,
            const std::vector<ParticleData>& particles);

    private:
        void resolveMaterials(
            model::NodeType* type,
            mesh::LodMesh& lodMesh,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            const LodData* lodData);

        void resolveMeshes(
            model::NodeType* type,
            const NodeTypeData& typeData);

        void resolveAddonMeshes(
            model::NodeType* type,
            const NodeTypeData& typeData);

        void resolveMesh(
            model::NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData);

        // @return count of meshes added
        int resolveModelMesh(
            model::NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData);

        void resolveLodMesh(
            model::NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        const LodData* resolveLod(
            model::NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        void resolveRig(
            const MeshData& meshData,
            animation::Rig* rig);

        void resolveSockets(
            const MeshData& meshData,
            animation::Rig* rig);

        void resolveAnimations(
            const MeshData& meshData,
            animation::Rig* rig);

        void loadAnimation(
            const std::string rootDir,
            const std::string& baseDir,
            animation::Rig& rig,
            const AnimationData& data);

        //void resolveAttachments(
        //    model::NodeTypeNodeType* type,
        //    const NodeTypeD
        // ata& typeData);

        void assignTypeFlags(
            const NodeTypeData& typeData,
            TypeFlags& flags);

        void assignMeshFlags(
            const FlagContainer& container,
            mesh::MeshFlags& flags);

    private:
        std::shared_ptr<Context> m_ctx;
        std::shared_ptr<Loaders> m_loaders;
    };
}
