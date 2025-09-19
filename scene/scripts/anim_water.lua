local cmd = self.cmd

local function animation(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=3 })

    cid = cmd:move_spline(
      { after=wid, time=10, relative=true },
      vec3(0, 1.5, 0),
      vec3(0, -2.5, 0))

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:move_spline(
      { after=wid, time=10, relative=false },
      vec3(0, -1.5, 0),
      orig_pos)

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
