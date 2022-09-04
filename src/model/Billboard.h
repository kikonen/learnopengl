#pragma once

#include "glm/glm.hpp"
#include "Node.h"

class Billboard : public Node
{
public:
	Billboard(std::shared_ptr<NodeType> type);
};

