#include "YamlConverter.h"

#include <fstream>

#include <fmt/format.h>

#include "ki/yaml.h"

#include "DocNode.h"

namespace loader {
    YamlConverter::YamlConverter() = default;
    YamlConverter::~YamlConverter() = default;

    loader::DocNode YamlConverter::load(const std::string& filePath)
    {
        try {
            std::ifstream fin(filePath);
            YAML::Node yamlRoot = YAML::Load(fin);

            return convertNode("<root>", yamlRoot);
        }
        catch (const std::runtime_error& ex) {
            throw std::runtime_error{ fmt::format(
                "LOADER::YAML::PARSE_FAILED: {} - {}",
                filePath, ex.what()) };
        }
        catch (const std::exception& ex) {
            throw std::runtime_error{ fmt::format(
                "LOADER::YAML::PARSE_FAILED: {} - {}",
                filePath, ex.what()) };
        }
    }

    loader::DocNode YamlConverter::convertNode(
        const std::string& name,
        const YAML::Node& yamlNode)
    {
        loader::DocNode node{ name };

        const auto type = yamlNode.Type();

        if (type == YAML::NodeType::Map) {
            node.m_type = DocNodeType::map;
            for (const auto& pair : yamlNode) {
                const std::string& k = pair.first.as<std::string>();
                node.addNode(convertNode(k, pair.second));
            }
        }
        else if (type == YAML::NodeType::Sequence) {
            node.m_type = DocNodeType::sequence;
            int index = 0;
            for (const auto& e : yamlNode) {
                node.addNode(convertNode(std::to_string(index), e));
                index++;
            }
        }
        else if (type == YAML::NodeType::Scalar) {
            node.m_type = DocNodeType::scalar;
            node.setValue(yamlNode.as<std::string>());
        }
        else if (type == YAML::NodeType::Null) {
            node.m_type = DocNodeType::null;
        }
        else {
            throw std::runtime_error("LOADER::YAML::UNKNOWN_NODE_TYPE");
        }

        return node;
    }
}
