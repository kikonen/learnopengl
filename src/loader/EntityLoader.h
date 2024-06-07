#pragma once

#include <vector>

#include "BaseLoader.h"
#include "EntityRoot.h"

namespace loader {
    class EntityLoader : public BaseLoader
    {
    public:
        EntityLoader(
            Context ctx);

        void loadEntities(
            const loader::DocNode& node,
            std::vector<EntityRoot>& entities,
            Loaders& loaders) const;

        void loadEntity(
            const loader::DocNode& node,
            EntityRoot& entityRoot,
            Loaders& loaders) const;

        void loadEntityClone(
            const loader::DocNode& node,
            EntityData& entityData,
            std::vector<EntityData>& clones,
            bool recurse,
            Loaders& loaders) const;

        void loadText(
            const loader::DocNode& node,
            TextData& data) const;

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
