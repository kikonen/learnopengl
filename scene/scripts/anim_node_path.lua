local rnd = math.random

local function animation_translate(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0

  local center = node:get_pos(self.handle)

  --print(string.format("center: %d, %d, %d", center[1], center[2], center[3]))

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    local posX = 10 - rnd(20)
    local posY = 3 - rnd(6)
    local posZ = 5 - rnd(10)

    --print(string.format("move: %f, %f, %f", posX, posY, posZ))

    local pos = vec3(center.x + posX, center.y + posY, center.z + posZ)

    --printf("pos: %s\n", pos)

    cid = cmd:move_spline(
      self.handle,
      { after=wid, time=5, relative=false },
      vec3(rnd(20) - 10 , rnd(10) - 5, rnd(10) - 5),
      pos)

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

local function animation_rotate(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    local rotX = 180 - rnd(360)
    local rotY = 180 - rnd(360)
    local rotZ = 180 - rnd(360)

    cid1 = cmd:rotate(
      self.handle,
      { after=wid, time=5, relative=true },
      vec3(1, 0, 0),
      rotX)

    cid2 = cmd:rotate(
      self.handle,
      { after=wid, time=5, relative=true },
      vec3(0, 1, 0),
      rotY)

    cid3 = cmd:rotate(
      self.handle,
      { after=wid, time=5, relative=true },
      vec3(0, 0, 1),
      rotZ)

    cid = cmd:sync(
      { after=wid },
      { cid1, cid2, cid3 })

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

local function animation_scale(self)
  local listener_id = nil
  local orig_pos = node:get_pos(self.handle)
  local wid = 0
  local cid = 0

  local function animation_listener()
    wid = cmd:wait({ after=cid, time=0 })

    local scale = rnd(30) / 10.0

    if scale < 0.1 then
      scale = 0.1
    elseif scale > 2.0 then
      scale = 2.0
    end

    cid = cmd:scale(
      self.handle,
      { after=wid, time=5, relative=false },
      vec3(scale, scale, scale))

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

animation_translate(self)
animation_rotate(self)
animation_scale(self)
