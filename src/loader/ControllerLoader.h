#pragma once

#include "BaseLoader.h"

class Node;
class NodeController;

namespace loader {
    enum class ControllerType {
        none,
        camera,
    };

    struct ControllerData {
        bool enabled{ false };
        ControllerType type{ ControllerType::none };

        int mode{ 0 };
        float speed{ 0.f };
    };

    class ControllerLoader : public BaseLoader
    {
    public:
        ControllerLoader(
            Context ctx);

        void loadController(
            const YAML::Node& node,
            ControllerData& data);

        NodeController* createController(
            const ControllerData& data,
            Node* node);
    };
}
