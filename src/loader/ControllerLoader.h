#pragma once

#include "BaseLoader.h"
#include "ControllerData.h"

class Node;
class NodeController;

namespace loader {
    class ControllerLoader : public BaseLoader
    {
    public:
        ControllerLoader(
            Context ctx);

        void loadController(
            const YAML::Node& node,
            ControllerData& data) const;

        NodeController* createController(
            const ControllerData& data,
            Node* node);
    };
}
