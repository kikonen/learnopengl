#pragma once

#include "DocNode.h"
#include "NodeData.h"

namespace loader {
    //template<typename T>
    //T DocNode::as() const noexcept
    //{
    //    //if constexpr(std::is_same<T, std::string>) {
    //    //    return m_data->m_value;
    //    //}
    //}

    //template<typename T>
    //std::enable_if<std::is_same<T, bool>::value, T>::type DocNode::as()
    //{
    //    return m_data->asBool();
    //}

    //template<>
    //std::string DocNode::as() const noexcept
    //{
    //    return m_data->asString();
    //}

    //template<>
    //int DocNode::as() const noexcept
    //{
    //    return m_data->asInt();
    //}

    //template<>
    //float DocNode::as() const noexcept
    //{
    //    return m_data->asFloat();
    //}

    //template<>
    //bool DocNode::as() const noexcept
    //{
    //    return m_data->asBool();
    //}
}
