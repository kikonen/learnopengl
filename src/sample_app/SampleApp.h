#pragma once

#include "kigl/kigl.h"

#include "asset/Assets.h"

#include "engine/Engine.h"

#include "gui/FrameInit.h"

#include "TestSceneSetup.h"

#include "event/Listen.h"

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

    int onUpdate(const UpdateContext& ctx) override;
    int onRender(const ki::RenderClock& clock) override;
    int onPost(const UpdateContext& ctx) override;

    void onDestroy() override;

    virtual void showFps(const ki::FpsCounter& fpsCounter) override;

private:
    void processInput();

    void frustumDebug(
        const ki::RenderClock& clock);

    Assets loadAssets();

    void onShoot();
    void onLoadScene();

    std::shared_ptr<Scene> loadScene();
    void unloadScene();
    void stopLoader();

private:
    std::shared_ptr<FrameInit> m_editorFrameInit;
    std::shared_ptr<editor::EditorFrame> m_editorFrame;

    std::unique_ptr<loader::SceneLoader> m_loader;
    std::unique_ptr<TestSceneSetup> m_testSetup;

    event::Listen m_listen_action_editor_scene_load;
    event::Listen m_listen_scene_loaded;

    size_t m_drawCount = 0;
    size_t m_skipCount = 0;

    float m_frustumElapsedSecs = 0;
};
