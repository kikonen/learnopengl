#include "LuaCommand.h"

#include "script/NodeCommandAPI.h"

namespace script
{
    // https://thephd.dev/sol3-feature-complete
    void LuaCommand::bind(sol::state& lua)
    {
        sol::usertype<NodeCommandAPI> t = lua.new_usertype<NodeCommandAPI>("NodeCommand");

        t["cancel"] = &NodeCommandAPI::lua_cancel;
        t["wait"] = &NodeCommandAPI::lua_wait;
        t["sync"] = &NodeCommandAPI::lua_sync;

        t["move"] = &NodeCommandAPI::lua_move;
        t["move_spline"] = &NodeCommandAPI::lua_move_spline;
        t["rotate"] = & NodeCommandAPI::lua_rotate;
        t["scale"] = &NodeCommandAPI::lua_scale;

        t["set_text"] = & NodeCommandAPI::lua_set_text;
        t["set_visible"] = & NodeCommandAPI::lua_set_visible;
        t["select"] = &NodeCommandAPI::lua_select;

        t["audio_play"] = & NodeCommandAPI::lua_audio_play;
        t["audio_pause"] = & NodeCommandAPI::lua_audio_pause;
        t["audio_stop"] = & NodeCommandAPI::lua_audio_stop;

        t["particle_emit"] = & NodeCommandAPI::lua_particle_emit;
        t["particle_stop"] = & NodeCommandAPI::lua_particle_stop;

        t["animation_play"] = & NodeCommandAPI::lua_animation_play;

        t["call"] = & NodeCommandAPI::lua_invoke;
        t["emit"] = & NodeCommandAPI::lua_emit;
    }
}
