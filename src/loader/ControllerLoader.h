#pragma once

#include "BaseLoader.h"
#include "ControllerData.h"

namespace pool {
    struct NodeHandle;
}

namespace model
{
    class Node;
}

struct ControllerDefinition;
//class NodeController;

namespace loader {
    class ControllerLoader : public BaseLoader
    {
    public:
        ControllerLoader(
            const std::shared_ptr<Context>& ctx);

        void loadControllers(
            const loader::DocNode& node,
            std::vector<ControllerData>& controllers) const;

        void loadController(
            const loader::DocNode& node,
            ControllerData& data) const;

        std::unique_ptr<ControllerDefinition> createControllerDefinition(
            const ControllerData& data);
    };
}
