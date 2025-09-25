#pragma once

#include "kigl/kigl.h"

#include "asset/Assets.h"

#include "engine/Engine.h"

#include "gui/FrameInit.h"

#include "TestSceneSetup.h"

struct Material;

namespace loader {
    class SceneLoader;
}

namespace editor {
    class EditorFrame;
}

namespace physics {
    struct RayHit;
}

namespace render
{
    class RenderContext;
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
        const render::RenderContext& ctx,
        const ki::RenderClock& clock);

    void raycastPlayer(
        const render::RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState);

    void shoot(
        const render::RenderContext& ctx,
        Scene* scene,
        const Input& input,
        const InputState& inputState,
        const InputState& lastInputState);

    void shootCallback(
        const physics::RayHit& hit
    );

    Assets loadAssets();
    std::shared_ptr<Scene> loadScene();

private:
    std::shared_ptr<FrameInit> m_editorFrameInit;
    std::shared_ptr<editor::EditorFrame> m_editorFrame;

    std::vector<std::unique_ptr<loader::SceneLoader>> m_loaders;
    std::unique_ptr<TestSceneSetup> m_testSetup;

    size_t m_drawCount = 0;
    size_t m_skipCount = 0;

    float m_frustumElapsedSecs = 0;
};
