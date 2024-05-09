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
            const loader::Node& node,
            std::vector<EntityRoot>& entities,
            Loaders& loaders) const;

        void loadEntity(
            const loader::Node& node,
            EntityRoot& entityRoot,
            Loaders& loaders) const;

        void loadEntityClone(
            const loader::Node& node,
            EntityData& entityData,
            std::vector<EntityData>& clones,
            bool recurse,
            Loaders& loaders) const;

        void loadText(
            const loader::Node& node,
            TextData& data) const;

        void loadLods(
            const loader::Node& node,
            std::vector<LodData>& lods,
            Loaders& loaders) const;

        void loadLod(
            const loader::Node& node,
            LodData& data,
            Loaders& loaders) const;

        void loadMaterialReferences(
            const loader::Node& node,
            std::vector<MaterialReference>& references,
            Loaders& loaders) const;

        void loadMaterialReference(
            const loader::Node& node,
            MaterialReference& data,
            Loaders& loaders) const;

        void loadAnimations(
            const loader::Node& node,
            std::vector<AnimationData>& animations) const;

        void loadAnimation(
            const loader::Node& node,
            AnimationData& data) const;
    };
}
