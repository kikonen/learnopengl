local cmd = self.cmd

local clone_index = node:get_clone_index(self.handle)
local orig_pos = node:get_pos(self.handle)

local function animation_move(self)
  local listener_id = nil
  local wid = 0
  local cid = 0

  local dir = 1
  if clone_index % 2 == 0 then
    dir = -1
  end

  local speed = 30

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=2 })

    cid = cmd:move(
      { after=wid, time=speed, relative=true },
      vec3(dir * 10, 0, 0.0))

    cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -1 * dir
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

local function animation_rotate(self)
  local listener_id = nil
  local wid = 0
  local cid = 0
  local orig_pos = nil

  local dir = 1
  if clone_index % 2 == 0 then
    dir = -1
  end

  local speed = 120 / ((clone_index + 1) * 0.5)

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=2 })

    cid = cmd:rotate(
      { after=wid, time=speed, relative=true },
      vec3(0.0, 1.0, 0.0),
      dir * 360)

    cmd:emit(
      { after=cid },
      { type=Event.SCRIPT_RESUME, listener=listener_id})

    dir = -1 * dir
  end

  listener_id = self:listen(animation_listener, {Event.SCRIPT_RESUME})

  cmd:emit(
    {},
    { type=Event.SCRIPT_RESUME, listener=listener_id})
end

animation_move(self)
animation_rotate(self)
