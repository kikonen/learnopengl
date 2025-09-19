local cmd = self.cmd

local function animation(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0

  local dir = 1
  local scale = 600

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:move(
      { after=wid, time=30, relative=true },
      vec3(dir * scale, 0.0, 0.0))

    wid = cmd:wait({ after=cid, time=1 })

    cid = cmd:emit(
      { after=wid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -dir
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
