#include "LuaCommand.h"

#include "script/NodeCommandAPI.h"

namespace script
{
    void LuaCommand::bind(sol::state& state)
    {
        state.new_usertype<NodeCommandAPI>("NodeCommandApi",
            "cancel", &NodeCommandAPI::lua_cancel,
            "wait", &NodeCommandAPI::lua_wait,
            "sync", &NodeCommandAPI::lua_sync,

            "move", &NodeCommandAPI::lua_move,
            "move_spline", &NodeCommandAPI::lua_move_spline,
            "rotate", & NodeCommandAPI::lua_rotate,
            "scale", &NodeCommandAPI::lua_scale,

            "set_text", &NodeCommandAPI::lua_set_text,
            "set_visible", &NodeCommandAPI::lua_set_visible,

            "audio_play", &NodeCommandAPI::lua_audio_play,
            "audio_pause", &NodeCommandAPI::lua_audio_pause,
            "audio_stop", &NodeCommandAPI::lua_audio_stop,

            "particle_emit", &NodeCommandAPI::lua_particle_emit,
            "particle_stop", &NodeCommandAPI::lua_particle_stop,

            "animation_play", &NodeCommandAPI::lua_animation_play,

            "call", &NodeCommandAPI::lua_call,
            "emit", &NodeCommandAPI::lua_emit
        );
    }
}
