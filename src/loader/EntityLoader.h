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
    };
}
