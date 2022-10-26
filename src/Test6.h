#pragma once

#include "ki/GL.h"

#include "Engine.h"

#include "gui/FrameInit.h"
#include "gui/Frame.h"

#include "TestSceneSetup.h"
#include "SceneFile.h"


class Test6 final : public Engine {
public:
    Test6();

protected:
    int onInit() override;
    int onSetup() override;

    int onRender(const RenderClock& clock) override;
    void onDestroy() override;

private:
    void selectNode(
        const RenderContext& ctx,
        bool isShift,
        bool isCtrl);

    Assets loadAssets();
    std::shared_ptr<Scene> loadScene();

private:
    std::unique_ptr<Frame> frame;
    std::unique_ptr<FrameInit> frameInit;

    std::unique_ptr<SceneFile> file;
    std::unique_ptr<TestSceneSetup> testSetup;

    unsigned long drawCount = 0;
    unsigned long skipCount = 0;
    float frustumElapsedSecs = 0;
};
