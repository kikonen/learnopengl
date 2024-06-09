#pragma once

#include <vector>

#include "BaseLoader.h"

#include "MeshData.h"
#include "LodData.h"
#include "AnimationData.h"

namespace loader {
    class MeshLoader : public BaseLoader
    {
    public:
        MeshLoader(
            Context ctx);

        void loadMeshes(
            const loader::DocNode& node,
            std::vector<MeshData>& meshes,
            Loaders& loaders) const;

        void loadMesh(
            const loader::DocNode& node,
            MeshData& data,
            Loaders& loaders) const;

        void loadLods(
            const loader::DocNode& node,
            std::vector<LodData>& lods) const;

        void loadLod(
            const loader::DocNode& node,
            LodData& data) const;

        void loadAnimations(
            const loader::DocNode& node,
            std::vector<AnimationData>& animations) const;

        void loadAnimation(
            const loader::DocNode& node,
            AnimationData& data) const;
    };
}
