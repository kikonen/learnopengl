#include "DocNodeData.h"

#include "util/util.h"

#include "DocNode.h"

namespace {
    const loader::DocNode NULL_NODE{ loader::DocNodeType::null };
}

namespace loader {
    const loader::DocNode& DocNodeData::findNode(const std::string& key) const noexcept
    {
        const auto& it = std::find_if(
            m_nodes.cbegin(),
            m_nodes.cend(),
            [&key](const auto& node)
            {
                return node.m_name == key;
            });

        return it != m_nodes.end() ? *it : NULL_NODE;
    }

    //template<typename T>
    //auto NodeData::as() const noexcept
    //{
    //    return m_value;
    //}

    bool DocNodeData::asBool() const noexcept {
        //if (!util::isBool(m_data->m_value))
        //{
        //    KI_WARN(fmt::format("invalid bool={}", renderNode(*this)));
        //    return false;
        //}

        return util::readBool(m_value, false);
    }
}
