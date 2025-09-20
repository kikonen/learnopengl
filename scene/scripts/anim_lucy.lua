local function animation_rotate(self)
  local listener_id = nil
  local orig_pos = nil
  local wid = 0
  local cid = 0
  local dir = 1

  local AUDIO_WIND = util.sid("wind")
  local AUDIO_ROTATE = util.sid("rotate")

  -- prepause
  cid = cmd:wait({ after=cid, time=1 })

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:audio_play(
      self.handle,
      { after=cid, sid=AUDIO_ROTATE })

    cid = cmd:rotate(
      self.handle,
      { after=wid, time=40, relative=true },
      vec3(0, 1, 0),
      dir * 720 * 2)

    cid = cmd:audio_pause(
      self.handle,
      { after=cid, sid=AUDIO_ROTATE })

    cid = cmd:audio_play(
      self.handle,
      { after=cid, sid=AUDIO_WIND, sync=true })

    cid = cmd:emit(
      self.handle,
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -dir
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    self.handle,
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

local function animation_scale(self)
  local listener_id = nil
  local orig_pos = nil
  local wid = 0
  local cid = 0
  local dir = 1

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=1 })

    local scale = vec3(1, 1, 1)
    if dir < 0 then
      scale = vec3(4, 4, 4)
    end

    cid = cmd:scale(
      { after=wid, time=20, relative=false },
      scale)

    cid = cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -dir
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation_scale(self)
animation_rotate(self)
