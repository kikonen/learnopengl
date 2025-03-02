--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation()
  local listener_id = nil
  local orig_pos = node:get_pos()
  local wid = 0
  local cid = 0
  local dir = 1

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:rotate(
      { after=wid, time=3, relative=true },
      { 0, 1, 0 },
      dir * 45)

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -dir
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  wid = cmd:wait({ after=cid, time=1 })

  cid = cmd:emit(
    { after=wid },
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation()
