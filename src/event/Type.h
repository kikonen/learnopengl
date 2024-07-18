#pragma once

#include <type_traits>

namespace event
{
    enum class Type : std::underlying_type_t<std::byte> {
        none = 0,

        node_add,
        node_added,
        //node_change_parent,
        node_select,
        node_activate,

        type_prepare_view,

        // NOTE KI allow camera to vary independent of active node
        camera_activate,

        controller_add,

        audio_listener_add,
        audio_source_add,
        audio_listener_update,
        audio_source_update,
        audio_listener_activate,

        audio_source_play,
        audio_source_stop,
        audio_source_pause,

        physics_add,

        command_wait,
        command_move,
        command_rotate,

        scene_loaded,

        script_bind,
        script_run,

        app_shutdown,
    };
}
