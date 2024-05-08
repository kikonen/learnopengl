#pragma once

#include <vector>

#include "BaseLoader.h"
#include "EntityData.h"

namespace loader {
    class EntityLoader : public BaseLoader
    {
    public:
        EntityLoader(
            Context ctx);

        void loadEntities(
            const YAML::Node& node,
            std::vector<EntityData>& entities,
            Loaders& loaders) const;

        void loadEntity(
            const YAML::Node& node,
            EntityData& data,
            Loaders& loaders) const;

        void loadEntityClone(
            const YAML::Node& node,
            EntityCloneData& data,
            std::vector<EntityCloneData>& clones,
            bool recurse,
            Loaders& loaders) const;

        void loadText(
            const YAML::Node& node,
            TextData& data) const;

        void loadLods(
            const YAML::Node& node,
            std::vector<LodData>& lods,
            Loaders& loaders) const;

        void loadLod(
            const YAML::Node& node,
            LodData& data,
            Loaders& loaders) const;

        void loadMaterialReferences(
            const YAML::Node& node,
            std::vector<MaterialReference>& references,
            Loaders& loaders) const;

        void loadMaterialReference(
            const YAML::Node& node,
            MaterialReference& data,
            Loaders& loaders) const;

        void loadAnimations(
            const YAML::Node& node,
            std::vector<AnimationData>& animations) const;

        void loadAnimation(
            const YAML::Node& node,
            AnimationData& data) const;
    };
}
