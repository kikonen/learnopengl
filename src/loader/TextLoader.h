#pragma once

#include "BaseLoader.h"

#include "TextData.h"

#include "generator/NodeGenerator.h"

struct TextDefinition;
class NodeType;

namespace loader {
    class TextLoader : public BaseLoader
    {
    public:
        TextLoader(
            std::shared_ptr<Context> ctx);

        void loadText(
            const loader::DocNode& node,
            TextData& data,
            Loaders& loaders) const;

        std::unique_ptr<TextDefinition> createDefinition(
            const NodeType* type,
            const TextData& data,
            Loaders& loaders);
    };
}
