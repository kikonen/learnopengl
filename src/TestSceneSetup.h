#pragma once

#include "glm/glm.hpp"

#include "scene/AsyncLoader.h"

class TestSceneSetup final
{
public:
    TestSceneSetup(
        AsyncLoader* asyncLoader,
        const Assets& assets);

    void setup(std::shared_ptr<Scene> scene);

private:
    void setupNodeBrickwallBox();

    void setupEffectExplosion();

    void setupViewport1();
private:
    const Assets& assets;
    AsyncLoader* asyncLoader;

    std::shared_ptr<Scene> scene;
};
