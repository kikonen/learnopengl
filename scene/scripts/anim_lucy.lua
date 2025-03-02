--printf("START: name=%s, id=%d, clone=%d\n", node:get_name(), id, node:get_clone_index())

local function animation_rotate()
  local listener_id = nil
  local orig_pos = nil
  local wid = 0
  local cid = 0
  local dir = 1

  local AUDIO_WIND = util:sid("wind")
  local AUDIO_ROTATE = util:sid("rotate")

  -- prepause
  cid = cmd:wait({ after=cid, time=1 })

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:audio_play(
      { after=cid, sid=AUDIO_ROTATE })

    cid = cmd:rotate(
      { after=wid, time=40, relative=true },
      { 0, 1, 0 },
      dir * 720 * 2)

    cid = cmd:audio_pause(
      { after=cid, sid=AUDIO_ROTATE })

    cid = cmd:audio_play(
      { after=cid, sid=AUDIO_WIND, sync=true })

    cid = cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -dir
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

local function animation_scale()
  local listener_id = nil
  local orig_pos = nil
  local wid = 0
  local cid = 0
  local dir = 1

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=1 })

    local scale = { 1, 1, 1 }
    if dir < 0 then
      scale = { 4, 4, 4 }
    end

    cid = cmd:scale(
      { after=wid, time=20, relative=false },
      scale)

    cid = cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -dir
  end

  listener_id = events:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation_scale()
animation_rotate()
