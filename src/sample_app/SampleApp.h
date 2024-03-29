#pragma once

#include "kigl/kigl.h"

#include "asset/Assets.h"

#include "engine/Engine.h"

#include "gui/FrameInit.h"
#include "gui/Frame.h"

#include "TestSceneSetup.h"

namespace loader {
    class SceneLoader;
}

class SampleApp final : public Engine {
public:
    SampleApp();
    ~SampleApp();

protected:
    int onInit() override;
    int onSetup() override;

    int onUpdate(const ki::RenderClock& clock) override;
    int onRender(const ki::RenderClock& clock) override;
    int onPost(const ki::RenderClock& clock) override;

    void onDestroy() override;

    virtual void showFps(const ki::FpsCounter& fpsCounter) override;

private:
    void frustumDebug(
        const RenderContext& ctx,
        const ki::RenderClock& clock);

    void selectNode(
        const RenderContext& ctx,
        Scene* scene,
        const InputState& inputState,
        const InputState& lastInputState);

    Assets loadAssets();
    std::shared_ptr<Scene> loadScene();

private:
    std::unique_ptr<Frame> m_frame;
    std::unique_ptr<FrameInit> m_frameInit;

    std::vector<std::unique_ptr<loader::SceneLoader>> m_loaders;
    std::unique_ptr<TestSceneSetup> m_testSetup;

    size_t m_drawCount = 0;
    size_t m_skipCount = 0;

    float m_frustumElapsedSecs = 0;
};
