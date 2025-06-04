#pragma once

#include <vector>

#include "BaseLoader.h"

#include "MeshData.h"
#include "LodData.h"
#include "SocketData.h"
#include "AnimationData.h"

namespace loader {
    class MeshLoader : public BaseLoader
    {
    public:
        MeshLoader(
            std::shared_ptr<Context> ctx);

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
            std::vector<LodData>& lods,
            Loaders& loaders) const;

        void loadLod(
            const loader::DocNode& node,
            LodData& data,
            Loaders& loaders) const;

        void loadSockets(
            const loader::DocNode& node,
            std::vector<SocketData>& sockets) const;

        void loadSocket(
            const loader::DocNode& node,
            SocketData& data) const;

        void loadAnimations(
            const loader::DocNode& node,
            std::vector<AnimationData>& animations) const;

        void loadAnimation(
            const loader::DocNode& node,
            AnimationData& data) const;
    };
}
