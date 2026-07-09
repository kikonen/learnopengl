local function defaults(o)
  o = o or {}
  return {
    min = o.min or 2,
    max = o.max or 30,
    time = o.time or 5,
    delay = o.delay or 1,
  }
end

local function animation_scale(self)
  local listener_id = nil
  local orig_pos = nil
  local wid = 0
  local cid = 0
  local dir = 1

  self.bubble = defaults(self.bubble)

  printf(
    "BUBBLE: %d, %d, %d, %d\n",
    self.bubble.min, self.bubble.max, self.bubble.time, self.bubble.delay)

  local function animation_listener()
    local bubble = defaults(self.bubble)

    local scale = vec3(bubble.min)
    if dir < 0 then
      scale = vec3(bubble.max)
    end

    wid = cmd:wait({ after=cid, time=bubble.delay })

    cid = cmd:scale(
      self.handle,
      { after=wid, time=bubble.time, relative=false },
      scale)

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

animation_scale(self)
