#pragma once

#include <string>
#include <memory>

#include "BaseLoader.h"

struct Material;
struct TypeFlags;
class NodeType;

namespace pool {
    struct TypeHandle;
}

namespace mesh {
    struct MeshFlags;
    class MeshSet;
    struct LodMesh;
}

namespace loader {
    struct NodeTypeData;
    struct RootData;
    struct MaterialData;
    struct ScriptSystemData;
    struct NodeRoot;
    struct TextData;
    struct MeshData;
    struct LodData;
    struct AnimationData;
    struct ResolvedNode;
    struct MaterialData;
    struct DecalData;
    struct MaterialUpdaterData;

    struct FlagContainer;

    class NodeTypeBuilder
    {
    public:
        NodeTypeBuilder(
            std::shared_ptr<Context> ctx,
            std::shared_ptr<Loaders> loaders);

        ~NodeTypeBuilder();

        void createTypes(
            const std::vector<NodeTypeData>& types);

        pool::TypeHandle createType(
            const NodeTypeData& typeData);

    private:
        void resolveMaterials(
            NodeType* type,
            mesh::LodMesh& lodMesh,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            const LodData* lodData);

        void resolveMeshes(
            NodeType* type,
            const NodeTypeData& typeData);

        void resolveMesh(
            NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            int index);

        // @return count of meshes added
        int resolveModelMesh(
            NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            int index);

        void resolveLodMesh(
            NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        const LodData* resolveLod(
            NodeType* type,
            const NodeTypeData& typeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        void resolveSockets(
            const MeshData& meshData,
            mesh::MeshSet& meshSet);

        void loadAnimation(
            const std::string& baseDir,
            const AnimationData& data,
            mesh::MeshSet& meshSet);

        void resolveAttachments(
            NodeType* type,
            const NodeTypeData& typeData);

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
