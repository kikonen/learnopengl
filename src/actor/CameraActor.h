#pragma once

#include "Actor.h"

#include "component/Camera.h"

class MeshType;

class CameraActor : public Actor {
public:
    CameraActor(MeshType* type)
        : Actor(type)
    {}

    inline Camera& getCamera() {
        return m_camera;
    }

private:
    Camera m_camera;
};
