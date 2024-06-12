#pragma once

#include "BaseLoader.h"

#include "TextData.h"

namespace loader {
    class TextLoader : public BaseLoader
    {
    public:
        TextLoader(
            Context ctx);

        void loadText(
            const loader::DocNode& node,
            TextData& data,
            Loaders& loaders) const;

        std::unique_ptr<NodeGenerator> createGenerator(
            const TextData& data,
            mesh::MeshType* type,
            Loaders& loaders);
    };
}
