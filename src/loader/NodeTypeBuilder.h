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
    struct RootData;
    struct MaterialData;
    struct ScriptSystemData;
    struct NodeRoot;
    struct NodeData;
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
        NodeTypeBuilder(std::shared_ptr<Loaders> loaders);
        ~NodeTypeBuilder();

        pool::TypeHandle createType(
            const NodeData& nodeData,
            const std::string& nameSuffix);

    private:
        void resolveMaterials(
            NodeType* type,
            mesh::LodMesh& lodMesh,
            const NodeData& nodeData,
            const MeshData& meshData,
            const LodData* lodData);

        void resolveMeshes(
            NodeType* type,
            const NodeData& nodeData);

        void resolveMesh(
            NodeType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            int index);

        // @return count of meshes added
        int resolveModelMesh(
            NodeType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            int index);

        void resolveLodMesh(
            NodeType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        const LodData* resolveLod(
            NodeType* type,
            const NodeData& nodeData,
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
            const NodeData& nodeData);

        void assignTypeFlags(
            const NodeData& nodeData,
            TypeFlags& flags);

        void assignMeshFlags(
            const FlagContainer& container,
            mesh::MeshFlags& flags);

    private:
        std::shared_ptr<Loaders> m_loaders;
    };
}
