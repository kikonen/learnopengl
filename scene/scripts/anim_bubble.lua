local function animation_scale(self)
  local listener_id = nil
  local orig_pos = nil
  local wid = 0
  local cid = 0
  local dir = 1

  printf("ANIM: max_size: %s\n", self.max_size)

  local max_scale = self.max_size or 30;

  local function animation_listener()
    local scale = vec3(1, 1, 1)
    if dir < 0 then
      scale = vec3(max_scale)
    end

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:scale(
      self.handle,
      { after=wid, time=5, relative=false },
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
