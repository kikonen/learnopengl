#include "NodeData.h"

#include "util/Util.h"

#include "DocNode.h"

namespace {
    const loader::DocNode NULL_NODE;
}

namespace loader {
    const loader::Node& NodeData::findNode(const std::string& key) const noexcept
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

    bool NodeData::asBool() const noexcept {
        //if (!util::isBool(m_data->m_value))
        //{
        //    KI_WARN(fmt::format("invalid bool={}", renderNode(*this)));
        //    return false;
        //}

        return util::readBool(m_value, false);
    }
}
