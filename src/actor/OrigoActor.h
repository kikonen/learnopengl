#pragma once

#include "model/Node.h"

class MeshType;

class OrigoActor : public Node {
public:
    OrigoActor(MeshType* type)
        : Node(type)
    {}
};
