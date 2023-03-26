#pragma once

#pragma once

#include "Actor.h"

#include "component/Light.h"

class MeshType;

class LightActor : public Actor {
public:
    LightActor(MeshType* type)
        : Actor(type)
    {}

    inline Light& getLight() {
        return m_light;
    }

private:
    Light m_light;
};
