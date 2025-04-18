#pragma once

#include <vector>
#include <string>
#include <memory>

#include "base.h"

namespace loader {
    struct DocNodeData;

    class DocNode {
        friend class YamlConverter;
        friend struct DocNodeData;

    public:
        struct Iterator {
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = loader::DocNode;
            using pointer = loader::DocNode*;  // or also value_type*
            using reference = loader::DocNode&;  // or also value_type&
        };

        DocNode();
        DocNode(loader::DocNodeType type);
        DocNode(const std::string& name);
        ~DocNode();

        DocNode& operator=(const DocNode& o);

        std::string str() const noexcept;

        const loader::DocNode getNode() const noexcept {
            return *this;
        }

        const std::string& getName() const noexcept {
            return m_name;
        }

        const loader::DocNode& findNode(const std::string& key) const noexcept;
        const std::vector<loader::DocNode>& getNodes() const noexcept;

        void addNode(const loader::DocNode& node);

        void setValue(std::string value);

        bool isValid() const noexcept {
            return m_type != DocNodeType::undefined;
        }

        bool isNull() const noexcept
        {
            return m_type == DocNodeType::null;
        }

        bool isSequence() const noexcept
        {
            return m_type == DocNodeType::sequence;
        }

        bool isScalar() const noexcept {
            return m_type == DocNodeType::scalar;
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
        std::string m_name;
        DocNodeType m_type{ DocNodeType::undefined };
        std::shared_ptr<DocNodeData> m_data;
    };
}
