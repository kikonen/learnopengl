#pragma once

#include "kigl/kigl.h"

#include "asset/Assets.h"

#include "util/Ref.h"

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

namespace game
{
    class Player;
}

class SampleApp final : public Engine {
public:
    SampleApp();
    ~SampleApp();

protected:
    bool onInit() override;
    bool onSetup() override;

    bool onUpdate(const UpdateContext& ctx) override;
    bool onRender(const ki::RenderClock& clock) override;

    void onDestroy() override;

    virtual void showFps(const ki::FpsCounter& fpsCounter) override;

private:
    void processInput();

    void frustumDebug(
        const ki::RenderClock& clock);

    void onShoot();

    void onLoadScene(const std::string& filePath);
    void onUnloadScene();

    util::Ref<Scene> loadScene(const std::string& filePath);
    void unloadScene();
    void stopLoader();
    void stopUpdaters();

private:
    std::shared_ptr<FrameInit> m_editorFrameInit;
    util::Ref<editor::EditorFrame> m_editorFrame;

    std::unique_ptr<loader::SceneLoader> m_loader;
    std::unique_ptr<TestSceneSetup> m_testSetup;

    event::Listen m_listen_action_editor_scene_load;
    event::Listen m_listen_action_editor_scene_unload;
    event::Listen m_listen_scene_loaded;
    event::Listen m_listen_scene_unload;

    util::Ref<game::Player> m_player;

    size_t m_drawCount = 0;
    size_t m_skipCount = 0;

    float m_frustumElapsedSecs = 0;
};
