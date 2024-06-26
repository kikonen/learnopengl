#include "YamlConverter.h"

#include <fstream>

#include "ki/yaml.h"

#include "DocNode.h"

namespace loader {
    YamlConverter::YamlConverter() = default;
    YamlConverter::~YamlConverter() = default;

    loader::DocNode YamlConverter::load(const std::string& filePath)
    {
        std::ifstream fin(filePath);
        YAML::Node yamlRoot = YAML::Load(fin);

        return convertNode("<root>", yamlRoot);
    }

    loader::DocNode YamlConverter::convertNode(
        const std::string& name,
        const YAML::Node& yamlNode)
    {
        loader::DocNode node{ name };

        const auto type = yamlNode.Type();

        if (type == YAML::NodeType::Map) {
            node.m_type = NodeType::map;
            for (const auto& pair : yamlNode) {
                const std::string& k = pair.first.as<std::string>();
                node.addNode(convertNode(k, pair.second));
            }
        }
        else if (type == YAML::NodeType::Sequence) {
            node.m_type = NodeType::sequence;
            int index = 0;
            for (const auto& e : yamlNode) {
                node.addNode(convertNode(std::to_string(index), e));
                index++;
            }
        }
        else if (type == YAML::NodeType::Scalar) {
            node.m_type = NodeType::scalar;
            node.setValue(yamlNode.as<std::string>());
        }
        else if (type == YAML::NodeType::Null) {
            node.m_type = NodeType::null;
        }
        else {
            throw "Unknown node type";
        }

        return node;
    }
}
