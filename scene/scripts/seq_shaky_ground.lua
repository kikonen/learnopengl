local cmd = self.cmd

local function animation(self)
  local listener_id
  local wid = 0
  local cid = 0

  local dir = 1

  local count = 0
  local angle = 5
  local time = 3

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0.5 })

    cid = cmd:rotate(
      { after=wid, time=time, relative=true },
      vec3(0, 0, 1),
      dir * angle )

    cid = cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    if count == 0 then
      angle = angle * 2
      time = time * 2
    end
    dir = -dir
    count = count + 1
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation(self)
