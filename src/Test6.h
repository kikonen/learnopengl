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

    int onRender(const ki::RenderClock& clock) override;
    void onDestroy() override;

private:
    void selectNode(
        const RenderContext& ctx,
        Scene* scene,
        bool isShift,
        bool isCtrl);

    Assets loadAssets();
    std::shared_ptr<Scene> loadScene();

private:
    std::unique_ptr<Frame> m_frame;
    std::unique_ptr<FrameInit> m_frameInit;

    std::vector<std::unique_ptr<SceneFile>> m_files;
    std::unique_ptr<TestSceneSetup> m_testSetup;

    size_t m_drawCount = 0;
    size_t m_skipCount = 0;

    float m_frustumElapsedSecs = 0;
};
