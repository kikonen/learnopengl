--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function attack(wid)
  --print(string.format("START: %d", id))
  return cid;
end

local function animation()
  local listener_id
  local wid = 0
  local cid = 0
  local orig_pos = node:get_pos()

  local function animation_listener()
    cid = cmd:animation_play(
      { after=wid, name = "idle:Unreal Take" } )

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:animation_play(
      { after=wid, name = "run:Unreal Take" } )

    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:animation_play(
      { after=wid, speed=0.5, name = "fire:Unreal Take" } )

    wid = cmd:wait({ after=cid, time=3 })

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
