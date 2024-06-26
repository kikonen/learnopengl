#pragma once

#include "BaseLoader.h"

#include "TextData.h"

#include "generator/NodeGenerator.h"


namespace mesh {
    class MeshType;
}

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
            const mesh::MeshType* type,
            const TextData& data,
            Loaders& loaders);
    };
}
