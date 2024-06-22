#pragma once

#include <memory>
#include <string>

#include "base.h"

namespace YAML {
    class Node;
}

namespace loader {
    // Convert YAML into internal format to allow adding support for
    // other input formats (read: JSON or such)
    class YamlConverter {
    public:
        YamlConverter();
        ~YamlConverter();

        loader::DocNode load(const std::string& filePath);
    private:
        loader::DocNode convertNode(
            const std::string& name,
            const YAML::Node& yamlNode);
    };
}
