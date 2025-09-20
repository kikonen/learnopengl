local function animation(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=5 })

    cid = cmd:rotate(
      self.handle,
      { after=wid, time=600, relative=true },
      vec3(0.0, 1.0, 0.0),
      360.0)

    wid = cmd:wait({ after=cid, time=0 })

    cid = cmd:emit(
      self.handle,
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    self.handle,
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
