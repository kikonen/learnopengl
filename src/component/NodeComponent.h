#pragma once

#include "ki/size.h"

template <class T>
struct NodeComponent
{
    ki::node_id m_nodeId;
    T m_component;
};
