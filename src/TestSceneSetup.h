#pragma once

#include "glm/glm.hpp"

#include "scene/AsyncLoader.h"

class TestSceneSetup final
{
public:
    TestSceneSetup(
        std::shared_ptr<AsyncLoader> asyncLoader,
        const std::shared_ptr<Assets> assets);
    ~TestSceneSetup();

    void setup(std::shared_ptr<Scene> scene);

private:
    void setupCamera();

    void setupNodeSkybox();

    void setupLightDirectional();
    void setupLightMoving();

    void setupNodeZero();

    void setupNodeWindow1();
    void setupNodeWindow2();
    void setupNodeStainedWindows();

    void setupNodeBrickwall();
    void setupNodeBigMirror();

    void setupNodeBrickwallBox();

    void setupNodeSpyro();

    void setupNodeBackpack();
    void setupNodeBunny();
    void setupNodeDragon();
    void setupNodeSpaceShuttle();
    void setupNodeSword2();

    void setupNodeTeapot();
    void setupNodeCow();
    void setupNodeBall();
    void setupNodeCube4();

    void setupNodeActive();
    void setupNodeBrickCube();

    void setupNodeMountains();

    void setupNodeGlassBall();
    void setupNodeMirrorBall();

    void setupNodeWaterBall();

    void setupNodeMaterialBalls();

    void setupNodePlanet();
    void setupNodeAsteroid();
    void setupNodeAsteroidBelt();

    void setupSpriteSkeleton();

    void setupTerrain();

    void setupWaterBottom();
    void setupWaterSurface();

    void setupSculpture1();

    void setupEffectExplosion();

    void setupViewport1();
private:
    std::shared_ptr<AsyncLoader> asyncLoader;
    std::shared_ptr<Assets> assets;

    std::shared_ptr<Scene> scene;
};
