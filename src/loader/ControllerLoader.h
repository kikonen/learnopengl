#pragma once

#include "BaseLoader.h"
#include "ControllerData.h"

namespace pool {
    struct NodeHandle;
}

class Node;
class NodeController;

namespace loader {
    class ControllerLoader : public BaseLoader
    {
    public:
        ControllerLoader(
            std::shared_ptr<Context> ctx);

        void loadControllers(
            const loader::DocNode& node,
            std::vector<ControllerData>& controllers) const;

        void loadController(
            const loader::DocNode& node,
            ControllerData& data) const;

        NodeController* createController(
            const ControllerData& data,
            pool::NodeHandle node);
    };
}
