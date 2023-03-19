#pragma once

#include "component/Light.h"
#include "component/Camera.h"
#include "component/ParticleGenerator.h"


union Component {
    Component() {};
    ~Component() {};

    Camera camera;
    Light light;
    ParticleGenerator particleGenerator;
};
