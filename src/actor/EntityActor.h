#pragma once

#pragma once

#include "Actor.h"

class MeshType;

class EntityActor : public Actor {
public:
    EntityActor(MeshType* type)
        : Actor(type)
    {}

private:
};
