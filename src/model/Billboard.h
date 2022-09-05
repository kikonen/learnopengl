#pragma once

#include "glm/glm.hpp"
#include "Node.h"

class Billboard final : public Node
{
public:
	Billboard(std::shared_ptr<NodeType> type);

	virtual ~Billboard();
};

