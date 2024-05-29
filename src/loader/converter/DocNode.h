#pragma once

#include <vector>
#include <string>
#include <memory>

#include "base.h"

namespace loader {
    struct NodeData;

    class DocNode {
        friend class YamlConverter;
        friend struct NodeData;

    public:
        struct Iterator {
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = loader::Node;
            using pointer = loader::Node*;  // or also value_type*
            using reference = loader::Node&;  // or also value_type&
        };

        DocNode();
        DocNode(loader::NodeType type);
        DocNode(const std::string& name);
        ~DocNode();

        std::string str() const noexcept;

        const loader::Node getNode() const noexcept {
            return *this;
        }

        const std::string& getName() const noexcept {
            return m_name;
        }

        const loader::Node& findNode(const std::string& key) const noexcept;
        const std::vector<loader::Node>& getNodes() const noexcept;

        void addNode(const loader::Node& node);

        void setValue(std::string value);

        bool isValid() const noexcept {
            return m_type != NodeType::undefined;
        }

        bool isSequence() const noexcept
        {
            return m_type == NodeType::sequence;
        }

        bool isScalar() const noexcept {
            return m_type == NodeType::scalar;
        }

        //template<typename T>
        //T as() const noexcept;

        const std::string& asString() const noexcept;
        int asInt() const noexcept;
        float asFloat() const noexcept;
        bool asBool() const noexcept;

    private:
        void createIfNeeded();

    private:
        const std::string m_name;
        NodeType m_type{ NodeType::undefined };
        std::shared_ptr<NodeData> m_data;
    };
}
