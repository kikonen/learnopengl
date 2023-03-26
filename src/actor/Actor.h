#pragma once

#include "model/Node.h"

class MeshType;

class Actor : public Node {
public:
    Actor(MeshType* type)
        : Node(type)
    {}
};
