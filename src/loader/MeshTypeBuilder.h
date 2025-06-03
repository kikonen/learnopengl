#pragma once

#include <string>
#include <memory>

#include "BaseLoader.h"

struct Material;

namespace pool {
    struct TypeHandle;
}

namespace mesh {
    struct TypeFlags;
    struct MeshFlags;
    class MeshSet;
    class MeshType;
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

    class MeshTypeBuilder
    {
    public:
        MeshTypeBuilder(std::shared_ptr<Loaders> loaders);
        ~MeshTypeBuilder();

        pool::TypeHandle createType(
            const NodeData& nodeData,
            const std::string& nameSuffix);

    private:
        void resolveMaterials(
            mesh::MeshType* type,
            mesh::LodMesh& lodMesh,
            const NodeData& nodeData,
            const MeshData& meshData,
            const LodData* lodData);

        void resolveMeshes(
            mesh::MeshType* type,
            const NodeData& nodeData);

        void resolveMesh(
            mesh::MeshType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            int index);

        // @return count of meshes added
        int resolveModelMesh(
            mesh::MeshType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            int index);

        void resolveLodMesh(
            mesh::MeshType* type,
            const NodeData& nodeData,
            const MeshData& meshData,
            mesh::LodMesh& lodMesh);

        const LodData* resolveLod(
            mesh::MeshType* type,
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
            mesh::MeshType* type,
            const NodeData& nodeData);

        void assignTypeFlags(
            const NodeData& nodeData,
            mesh::TypeFlags& flags);

        void assignMeshFlags(
            const FlagContainer& container,
            mesh::MeshFlags& flags);

    private:
        std::shared_ptr<Loaders> m_loaders;
    };
}
