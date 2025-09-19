local cmd = self.cmd

local function animation(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0
  local idx = 0

  local function animation_listener()
    cid = cmd:move({ after=cid, time=0, relative=false }, vec3(2, 0, -1))
    cid = cmd:move({ after=cid, time=10, relative=true }, vec3(-4, 0, 0))
    cid = cmd:move({ after=cid, time=10, relative=true }, vec3(4, 0, 0))

    cid = cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
