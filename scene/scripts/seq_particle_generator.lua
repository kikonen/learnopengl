printf("PARTICLE_GENERATOR: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local rnd = math.random

local function animation()
  local listener_id = nil
  local wid = 0
  local cid = 0

  local function animation_listener()
    cid = cmd:particle_emit(
      { after=wid, count=(30 + rnd(50)) * 100 })

    wid = cmd:wait(
      { after=cid, time=3 })

    wid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation()
