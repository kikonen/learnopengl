#include "DocNode.h"

#include <sstream>
#include <algorithm>

#include <fmt/format.h>

#include "util/Log.h"
#include "util/util.h"

#include "DocNodeData.h"

namespace {
    const loader::DocNode NULL_NODE{ loader::DocNodeType::null };
    const std::vector<loader::DocNode> NULL_NODES;

    std::string renderNode(
        const loader::DocNode& v) noexcept
    {
        std::stringstream ss;
        ss << v.str();
        return ss.str();
    }
}

namespace loader {
    DocNode::DocNode() = default;
    DocNode::~DocNode() = default;

    DocNode::DocNode(loader::DocNodeType type)
        : m_type{ type }
    {}

    DocNode::DocNode(const std::string& name)
        : m_name{ name }
    {}

    DocNode& DocNode::operator=(const DocNode& o)
    {
        m_name = o.m_name;
        m_type = o.m_type;
        m_data = o.m_data;
        return *this;
    }

    std::string DocNode::str() const noexcept {
        if (m_data) {
            return fmt::format("<name={}, value={}>", m_name, m_data->m_value);
        }
        return fmt::format("<name={}, value=NULL>", m_name);
    }

    const loader::DocNode& DocNode::findNode(const std::string& key) const noexcept
    {
        return m_data ? m_data->findNode(key) : NULL_NODE;
    }

    const std::vector<loader::DocNode>& DocNode::getNodes() const noexcept
    {
        return m_data ? m_data->m_nodes : NULL_NODES;
    }

    void DocNode::addNode(
        const loader::DocNode& node)
    {
        createIfNeeded();

        // NOTE KI for map latest entry in scanned document
        // MUST override old value
        for (int i = 0; i < m_data->m_nodes.size(); i++) {
            loader::DocNode& old = m_data->m_nodes[i];
            if (old.m_name == node.m_name) {
                old = node;
                return;
            }
        }

        m_data->m_nodes.push_back(node);
    }

    void DocNode::setValue(std::string value)
    {
        createIfNeeded();
        m_data->m_value = value;
    }

    const std::string& DocNode::asString() const noexcept {
        return m_data->asString();
    }

    int DocNode::asInt() const noexcept {
        return m_data->asInt();
    }

    float DocNode::asFloat() const noexcept {
        return m_data->asFloat();
    }

    bool DocNode::asBool() const noexcept {
        return m_data->asBool();
    }

    void DocNode::createIfNeeded()
    {
        if (m_data) return;
        m_data = std::make_shared<DocNodeData>();
    }
}
