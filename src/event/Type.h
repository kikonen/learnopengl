#pragma once

#include <type_traits>

namespace event
{
    enum class Type : std::underlying_type_t<std::byte> {
        none = 0,

        // NOTE KI deferred std::function<void()>, carried in Attachment::task.
        // Runs at this dispatcher's drain point, ordered with events via the
        // single shared queue (per-producer FIFO).
        invoke,

        node_add,
        node_added,

        node_remove,
        node_removed,

        // NOTE KI final cleanup (WT => RT)
        node_dispose,

        // NOTE KI node visibility changed (WT => RT): update per-drawable hidden flag
        node_visible,

        node_select,
        node_activate,

        type_prepare_view,

        // NOTE KI allow camera to vary independent of active node
        camera_activate,
        camera_activate_next,

        scene_loaded,
        scene_unload,

        script_run,

        viewport_changed,

        console_execute,
        console_complete,

        app_shutdown,

        action_editor_scene_load,
        action_editor_scene_unload,
        action_app_quit,

        action_game_shoot,
    };
}
